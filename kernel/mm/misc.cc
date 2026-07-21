#include <pbos/kd/logger.h>
#include <kernel/generated/config.hh>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/kh/mm/init.hh>
#include <pbos/kh/mm/misc.hh>
#include <pbos/ki/kasan/impl.hh>
#include <pbos/ki/km/symbol.hh>
#include <pbos/ki/mm/pgalloc.hh>
#include <pbos/ki/ps/proc.hh>

PBOS_FORCEINLINE uintptr_t ki_compute_reversal_mapping_key(size_t page_size, uintptr_t addr) {
	return kfxx::floor_align_to<uintptr_t>(addr, page_size * KI_REVERSAL_MAP_GRANULE);
}

mm_context_t **mm_cur_contexts = nullptr;
const ki_paging_config_t *ki_cur_paging_config;

ps::rec_mutex_t ki_kernel_mmap_mutex;

_mm_context_t::_mm_context_t() {
}

PBOS_PRIVATE ki_mm_rmlt_t::ki_mm_rmlt_t(kfxx::allocator_t *allocator) : vmrs(allocator) {
}

PBOS_PRIVATE ki_mm_rmlt_t::~ki_mm_rmlt_t() {
}

PBOS_PURE PBOS_API size_t mm_get_page_size() {
	return kh_get_page_size();
}

PBOS_PRIVATE km_result_t ki_mm_add_vmr_to_rmlt(ki_mm_rmlt_t *rmlt, mm_vmr_t *vmr) {
	// ps::mutex_guard g(rmlt->mutex);
	if (!rmlt->vmrs.insert_or_keep(+vmr))
		return KM_RESULT_NO_MEM;
	return KM_RESULT_OK;
}

PBOS_PRIVATE bool ki_mm_remove_vmr_from_rmlt(ki_mm_rmlt_t *rmlt, mm_vmr_t *vmr) {
	ps::mutex_guard g(rmlt->mutex);

	rmlt->vmrs.remove(vmr);
	if (!rmlt->vmrs.size()) {
		ki_mm_destroy_rmlt(rmlt);
		return true;
	}
	return false;
}

PBOS_PRIVATE ki_mm_rmlt_t *ki_mm_alloc_rmlt() {
	return kfxx::alloc_and_construct<ki_mm_rmlt_t>(kfxx::kernel_allocator(), kfxx::kernel_allocator());
}

PBOS_PRIVATE void ki_mm_destroy_rmlt(ki_mm_rmlt_t *rmlt) {
	kd_assert(!rmlt->vmrs.size());

	kfxx::destroy_and_release<ki_mm_rmlt_t>(kfxx::kernel_allocator(), rmlt);
}

PBOS_PRIVATE void ki_mm_destroy_vmr(mm_vmr_t *vmr) {
	size_t page_size = mm_get_page_size();
	for (size_t k = 0, j = 0; k < vmr->size; k += page_size * KI_REVERSAL_MAP_GRANULE, ++j) {
		if (vmr->get_index_alloc_mask(j)) {
			kfxx::destroy_at<ki_vmr_index_t>(&vmr->indices[j]);
		}
	}
	if (vmr->indices)
		kima_free(&*vmr->mm_context->kima_vmr_pool, vmr->indices);
	if (vmr->index_alloc_masks)
		kima_free(&*vmr->mm_context->kima_vmr_pool, vmr->index_alloc_masks);
	kfxx::destroy_at<mm_vmr_t>(vmr);
	kima_free(&*vmr->mm_context->kima_vmr_pool, vmr);
}

kfxx::option_t<kfxx::radix_map_t<uintptr_t, ki_reversal_map_t, 3>> ki_reversal_mapping;
ps::semaphore_t ki_reversal_mapping_semaphore;

PBOS_API km_result_t mm_mmap(mm_context_t *ctxt,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_page_access_t access,
	mmap_flags_t flags) {
	if (!ctxt)
		return KM_RESULT_INVALID_ARGS;
	if (!size)
		return KM_RESULT_INVALID_ARGS;

	size_t page_size = kh_get_page_size();
	void *aligned_vaddr = (void *)kfxx::floor_align_to<uintptr_t>((uintptr_t)vaddr, page_size),
		 *aligned_paddr = (void *)kfxx::floor_align_to<uintptr_t>((uintptr_t)paddr, page_size);
	size_t aligned_size = kfxx::ceil_align_to<size_t>(size, page_size);
	char *vaddr_limit = (char *)aligned_vaddr + (size - 1);
	bool is_user_space = mm_is_user_space(aligned_vaddr);
	ps_proc_id_t pid = PS_PROC_ID_MAX;

	// Overflow is an error.
	if (UINTPTR_MAX - (uintptr_t)aligned_vaddr < aligned_size) {
		dbg_println(__func__, "Trying to unmap with address and size combination that will overflow");
		return KM_RESULT_INVALID_ARGS;
	}

	if (is_user_space !=
		mm_is_user_space(vaddr_limit)) {
		dbg_println(__func__, "Trying to unmap across kernel and user space");
		return KM_RESULT_INVALID_ARGS;
	}

	if ((!is_user_space) && (access & MM_PAGE_USER)) {
		dbg_println(__func__, "Trying to map kernel space with user accessible");
		return KM_RESULT_INVALID_ARGS;
	}

	mm_vmr_t *vmr = nullptr;

	kfxx::switchable_scope_guard unreserve_quota_guard([ctxt, aligned_size, page_size]() noexcept {
		ki_unreserve_page_quota(ctxt, aligned_size / page_size);
	},
		false);

	kfxx::switchable_scope_guard release_reversal_mappings_guard([ctxt, aligned_size, page_size, paddr, vmr, pid]() noexcept {
		if (vmr)
			km_panic("Reversal mapping removing guard is triggered with vmr == nullptr, please report this bug");
		const size_t limit = aligned_size / page_size;
		for (size_t i = 0; i < limit; i += KI_REVERSAL_MAP_GRANULE) {
			ps::read_semaphore_guard gm_g(ki_reversal_mapping_semaphore);
			uintptr_t revmap_key = ki_compute_reversal_mapping_key(page_size, reinterpret_cast<uintptr_t>(paddr) + i);
			if (auto it = ki_reversal_mapping->find(revmap_key); it != ki_reversal_mapping->end()) {
				ps::read_semaphore_guard rvmr_g((*it).related_vmrs_semaphore);
				if (auto jt = (*it).proc_related_vmrs.find(pid); jt != (*it).proc_related_vmrs.end()) {
					ps::write_semaphore_guard vt_g((*jt).vmr_index_tree_semaphore);
					for (size_t k = 0, j = 0; k < vmr->size; k += page_size * KI_REVERSAL_MAP_GRANULE, ++j) {
						if (vmr->get_index_alloc_mask(j))
							(*jt).vmr_index_tree.remove(&vmr->indices[j]);
					}
					if (!(*jt).vmr_index_tree.size()) {
						(*it).proc_related_vmrs.remove(pid);
						vt_g.release();
					}
				}
				if (!(*it).proc_related_vmrs.size()) {
					ki_reversal_mapping->remove(revmap_key);
					rvmr_g.release();
				}
			}
		}

		ki_mm_destroy_vmr(vmr);
	},
		false);

	if (!(flags & MM_MMAP_IGNORE_VMR)) {
		if (is_user_space) {
			pid = ctxt->pcb->rb_value;
			if (flags & MM_MMAP_COMMIT_RESERVED_MEM) {
				ki_mm_lock_vmr(ctxt);
				kfxx::deferred lock_release([ctxt]() noexcept {
					ki_mm_unlock_vmr(ctxt);
				});

				if (auto node = ctxt->vmr_tree.find(aligned_vaddr); !node)
					return KM_RESULT_INVALID_ARGS;

				size_t i = 0;

				for (; i < size; i += page_size * KI_REVERSAL_MAP_GRANULE) {
					ki_reversal_map_t *m = nullptr;
					ps::read_semaphore_guard gm_g(ki_reversal_mapping_semaphore);
					uintptr_t revmap_key = ki_compute_reversal_mapping_key(page_size, reinterpret_cast<uintptr_t>(paddr) + i);
					if (auto it = ki_reversal_mapping->find(revmap_key); it != ki_reversal_mapping->end()) {
						m = &*it;
					} else
						km_panic("Kernel reversal mapping for %p is not found", static_cast<char *>(paddr) + i);

					ki_related_vmrs_entry_t *t = nullptr;
					ps::read_semaphore_guard rvmr_g(m->related_vmrs_semaphore);
					if (auto it = m->proc_related_vmrs.find(pid); it != m->proc_related_vmrs.end()) {
						t = &(*it);
					}
					km_panic("Process #%u's related VMR set of mapping for %p is not found", pid, static_cast<char *>(paddr) + i);
					gm_g.release();

					ps::read_semaphore_guard vt_g(t->vmr_index_tree_semaphore);

					kd_assert(vmr->index_alloc_cur_index < vmr->size / page_size);
					kfxx::construct_at<ki_vmr_index_t>(&vmr->indices[vmr->index_alloc_cur_index]);
					// Use itself as key, for uniqueness.
					vmr->indices[vmr->index_alloc_cur_index].rb_value = &vmr->indices[vmr->index_alloc_cur_index];
					t->vmr_index_tree.insert_unwrap(&vmr->indices[vmr->index_alloc_cur_index]);
					++vmr->index_alloc_cur_index;
				}
			} else {
				if (access & MM_PAGE_RESERVED) {
					if (paddr)
						return KM_RESULT_INVALID_ARGS;
					if (access & MM_PAGE_MAPPED)
						return KM_RESULT_INVALID_ARGS;
					KM_RETURN_IF_FAILED(mm_reserve_pages(ctxt, aligned_size / page_size));
					flags |= MM_MMAP_RESERVE_PGTAB_ONLY;
					unreserve_quota_guard.enable();
				}

				ki_mm_lock_vmr(ctxt);
				kfxx::deferred lock_release([ctxt]() noexcept {
					ki_mm_unlock_vmr(ctxt);
				});

				mm_vmr_t *vmr_begin, *vmr_end;

				vmr_begin = static_cast<mm_vmr_t *>(ctxt->vmr_tree.find_max_lteq(aligned_vaddr));
				vmr_end = static_cast<mm_vmr_t *>(ctxt->vmr_tree.find_max_lteq(vaddr_limit));

				if (ctxt->vmr_tree.size()) {
					// Check if vmr_begin and vmr_end has covered the area we are going to map.
					if (((vmr_begin) && (((char *)vmr_begin->rb_value) + (vmr_begin->size - 1) >= aligned_vaddr)))
						return KM_RESULT_INVALID_ARGS;
					if (((vmr_end) && (((char *)vmr_end->rb_value) + (vmr_end->size - 1) >= vaddr_limit)))
						return KM_RESULT_INVALID_ARGS;
				}

				mm_vmr_t *new_vmr = (mm_vmr_t *)kima_alloc(&*ctxt->kima_vmr_pool, sizeof(mm_vmr_t), alignof(mm_vmr_t));
				if (!new_vmr)
					return KM_RESULT_NO_MEM;
				kfxx::construct_at<mm_vmr_t>(new_vmr);

				new_vmr->rb_value = vaddr;
				new_vmr->size = size;
				new_vmr->access = access;
				new_vmr->mm_context = ctxt;

				kfxx::scope_guard destroy_new_vmr_guard([ctxt, new_vmr]() noexcept {
					ki_mm_destroy_vmr(new_vmr);
				});

				size_t index_len = kfxx::ceil_align_to(size, page_size * KI_REVERSAL_MAP_GRANULE) / KI_REVERSAL_MAP_GRANULE;
				if (!(new_vmr->index_alloc_masks = static_cast<uint8_t *>(kima_alloc(&*ctxt->kima_vmr_pool, index_len * sizeof(uint8_t), alignof(ki_vmr_index_t)))))
					return KM_RESULT_NO_MEM;
				// Initialize the index allocation mask with 0.
				memset(new_vmr->index_alloc_masks, 0, kfxx::ceil_align_to<size_t, 8>(index_len));
				if (!(new_vmr->indices = static_cast<ki_vmr_index_t *>(kima_alloc(&*ctxt->kima_vmr_pool, index_len * sizeof(ki_vmr_index_t), alignof(ki_vmr_index_t)))))
					return KM_RESULT_NO_MEM;

				ctxt->vmr_tree.insert_unwrap(new_vmr);

				vmr = new_vmr;

				destroy_new_vmr_guard.release();

				size_t i = 0;

				release_reversal_mappings_guard.enable();

				for (; i < size; i += page_size * KI_REVERSAL_MAP_GRANULE) {
					ki_reversal_map_t *m = nullptr;
					ps::read_semaphore_guard gm_g(ki_reversal_mapping_semaphore);
					uintptr_t revmap_key = ki_compute_reversal_mapping_key(page_size, reinterpret_cast<uintptr_t>(paddr) + i);
					if (auto it = ki_reversal_mapping->find(revmap_key); it != ki_reversal_mapping->end()) {
						m = &*it;
					} else {
						if (!ki_reversal_mapping->insert(revmap_key, ki_reversal_map_t(kfxx::kernel_allocator())))
							return KM_RESULT_NO_MEM;
						m = &ki_reversal_mapping->at(revmap_key);
					}

					ki_related_vmrs_entry_t *t = nullptr;
					ps::read_semaphore_guard rvmr_g(m->related_vmrs_semaphore);
					if (auto it = m->proc_related_vmrs.find(pid); it != m->proc_related_vmrs.end()) {
						t = &(*it);
					} else {
						if (!m->proc_related_vmrs.insert(pid, {}))
							return KM_RESULT_NO_MEM;
						t = &m->proc_related_vmrs.at(pid);
					}
					gm_g.release();

					kd_assert(vmr->index_alloc_cur_index < vmr->size / page_size);
					kfxx::construct_at<ki_vmr_index_t>(&vmr->indices[vmr->index_alloc_cur_index]);
					// Use itself as key, for uniqueness.
					vmr->indices[vmr->index_alloc_cur_index].rb_value = &vmr->indices[vmr->index_alloc_cur_index];
					t->vmr_index_tree.insert_unwrap(&vmr->indices[vmr->index_alloc_cur_index]);
					++vmr->index_alloc_cur_index;
				}
			}
		} else {
			if (access & MM_PAGE_RESERVED)
				return KM_RESULT_INVALID_ARGS;
		}
	}

	kfxx::scope_guard release_kernel_mmap_mutex_guard([]() noexcept {
		ki_kernel_mmap_mutex.unlock();
	});

	if (is_user_space)
		release_kernel_mmap_mutex_guard.release();
	else
		ki_kernel_mmap_mutex.lock();

	KM_RETURN_IF_FAILED(kh_mmap(ctxt, aligned_vaddr, aligned_paddr, aligned_size, access, flags));

#if KI_ENABLE_KASAN
	if (!(flags & MM_MMAP_IGNORE_KASAN)) {
		if (!is_user_space) {
			kfxx::scope_guard unmmap_guard([ctxt, aligned_vaddr, aligned_paddr, aligned_size]() noexcept {
				kh_munmap(ctxt, aligned_vaddr, aligned_size, 0);
			});

			KM_RETURN_IF_FAILED(ki_kasan_alloc_shadow_pages_for_vaddr(aligned_vaddr, aligned_size));

			unmmap_guard.release();
		}
	}
#endif

	release_reversal_mappings_guard.disable();
	unreserve_quota_guard.disable();

	return KM_RESULT_OK;
}

PBOS_API km_result_t mm_munmap(mm_context_t *ctxt, void *vaddr, size_t size, mmap_flags_t flags) {
	if (!ctxt)
		return KM_RESULT_INVALID_ARGS;

	void *aligned_vaddr = (void *)PGFLOOR(vaddr);
	size_t aligned_size = PGCEIL(size);
	char *vaddr_limit = (char *)aligned_vaddr + (size - 1);
	bool is_user_space = mm_is_user_space(aligned_vaddr);
	size_t page_size = kh_get_page_size();
	ps_proc_id_t pid = PS_PROC_ID_MAX;

	// Overflow is an error.
	if (UINTPTR_MAX - (uintptr_t)aligned_vaddr < aligned_size) {
		dbg_println(__func__, "Trying to unmap with address and size combination that will overflow");
		return KM_RESULT_INVALID_ARGS;
	}

	if (is_user_space !=
		mm_is_user_space(vaddr_limit)) {
		dbg_println(__func__, "Trying to unmap across kernel and user space");
		return KM_RESULT_INVALID_ARGS;
	}

	if (!(flags & MM_MUNMAP_IGNORE_VMR)) {
		if (is_user_space) {
			pid = ctxt->pcb->rb_value;

			ki_mm_lock_vmr(ctxt);
			kfxx::deferred lock_release([ctxt]() noexcept {
				ki_mm_unlock_vmr(ctxt);
			});

			struct user_data_t {
				mm_vmr_t *vmr;
				ps_proc_id_t pid;
				size_t page_size;
			};
			user_data_t user_data;

			mm_vmr_t *vmr = static_cast<mm_vmr_t *>(ctxt->vmr_tree.find(vaddr));

			if (!vmr)
				return KM_RESULT_INVALID_ARGS;

			if (size)
				return KM_RESULT_INVALID_ARGS;

			user_data = { vmr, pid, page_size };

			kh_walk_pgtab(
				ctxt,
				vaddr,
				size,
				[](void *vaddr, void *paddr, mm_page_access_t page_access, void *user_data) -> kf_control_flow_t {
					if (!(page_access & MM_PAGE_MAPPED))
						return KF_CONTROL_FLOW_CONTINUE;

					mm_vmr_t *vmr = static_cast<user_data_t *>(user_data)->vmr;

					ps::read_semaphore_guard gm_g(ki_reversal_mapping_semaphore);
					uintptr_t revmap_key = ki_compute_reversal_mapping_key(static_cast<user_data_t *>(user_data)->page_size, reinterpret_cast<uintptr_t>(paddr));
					if (auto it = ki_reversal_mapping->find(revmap_key); it != ki_reversal_mapping->end()) {
						ps::read_semaphore_guard rvmr_g((*it).related_vmrs_semaphore);
						if (auto jt = (*it).proc_related_vmrs.find(static_cast<user_data_t *>(user_data)->pid); jt != (*it).proc_related_vmrs.end()) {
							ps::write_semaphore_guard vt_g((*jt).vmr_index_tree_semaphore);
							for (size_t k = 0, j = 0; k < vmr->size; k += static_cast<user_data_t *>(user_data)->page_size, ++j) {
								if (vmr->get_index_alloc_mask(j))
									(*jt).vmr_index_tree.remove(&vmr->indices[j]);
							}
							if (!(*jt).vmr_index_tree.size()) {
								(*it).proc_related_vmrs.remove(static_cast<user_data_t *>(user_data)->pid);
								vt_g.release();
							}
						}
						if (!(*it).proc_related_vmrs.size()) {
							ki_reversal_mapping->remove(revmap_key);
							rvmr_g.release();
						}
					}
					return KF_CONTROL_FLOW_CONTINUE;
				},
				&user_data,
				KH_WALK_PGTAB_SKIP_UNMAPPED);

			aligned_size = PGCEIL(vmr->size);

			ctxt->vmr_tree.remove(vmr);

			ki_unreserve_page_quota(ctxt, vmr->size / page_size);
			ki_mm_destroy_vmr(vmr);
		}
	} else {
		if (!size)
			return KM_RESULT_INVALID_ARGS;
	}

	kfxx::scope_guard release_kernel_mmap_mutex_guard([]() noexcept {
		ki_kernel_mmap_mutex.unlock();
	});

	if (is_user_space)
		release_kernel_mmap_mutex_guard.release();
	else
		ki_kernel_mmap_mutex.lock();

	kh_munmap(ctxt, aligned_vaddr, aligned_size, flags);

#if KI_ENABLE_KASAN
	if (!(flags & MM_MMAP_IGNORE_KASAN)) {
		if (!is_user_space) {
			ki_kasan_scan_and_recycle_shadow_pages(aligned_vaddr, aligned_size);
		}
	}
#endif

	return KM_RESULT_OK;
}

PBOS_NODISCARD PBOS_API km_result_t mm_merge_mapped_area(
	mm_context_t *ctxt,
	void *vaddr_a,
	void *vaddr_b) {
	if (vaddr_a >= vaddr_b)
		return KM_RESULT_INVALID_ARGS;

	ki_mm_lock_vmr(ctxt);
	kfxx::deferred lock_release([ctxt]() noexcept {
		ki_mm_unlock_vmr(ctxt);
	});

	mm_vmr_t *vmr_a, *vmr_b;

	vmr_a = static_cast<mm_vmr_t *>(ctxt->vmr_tree.find(vaddr_a));
	if (!vmr_a) {
		dbg_println(__func__, "VMR not found with address %p", vaddr_a);
		return KM_RESULT_INVALID_ARGS;
	}

	vmr_b = static_cast<mm_vmr_t *>(ctxt->vmr_tree.find(vaddr_b));
	if (!vmr_b) {
		dbg_println(__func__, "VMR not found with address %p", vaddr_b);
		return KM_RESULT_INVALID_ARGS;
	}

	if (((char *)vmr_a->rb_value) + vmr_a->size != vmr_b->rb_value) {
		dbg_println(__func__, "VMR at %p is not neighbor of VMR %p", vaddr_a, vaddr_b);
		return KM_RESULT_INVALID_ARGS;
	}

	size_t page_size = kh_get_page_size();
	ps_proc_id_t pid = ctxt->pcb->rb_value;

	struct user_data_t {
		mm_vmr_t *vmr;
		ps_proc_id_t pid;
		size_t page_size;
	};
	user_data_t user_data;

	user_data = { vmr_b, pid, page_size };

	kh_walk_pgtab(
		ctxt,
		vmr_b->rb_value,
		vmr_b->size,
		[](void *vaddr, void *paddr, mm_page_access_t page_access, void *user_data) -> kf_control_flow_t {
			if (!(page_access & MM_PAGE_MAPPED))
				return KF_CONTROL_FLOW_CONTINUE;

			mm_vmr_t *vmr = static_cast<user_data_t *>(user_data)->vmr;

			ps::read_semaphore_guard gm_g(ki_reversal_mapping_semaphore);
			uintptr_t revmap_key = ki_compute_reversal_mapping_key(static_cast<user_data_t *>(user_data)->page_size, reinterpret_cast<uintptr_t>(paddr));
			if (auto it = ki_reversal_mapping->find(revmap_key); it != ki_reversal_mapping->end()) {
				ps::read_semaphore_guard rvmr_g((*it).related_vmrs_semaphore);
				if (auto jt = (*it).proc_related_vmrs.find(static_cast<user_data_t *>(user_data)->pid); jt != (*it).proc_related_vmrs.end()) {
					ps::write_semaphore_guard vt_g((*jt).vmr_index_tree_semaphore);
					for (size_t k = 0, j = 0; k < vmr->size; k += static_cast<user_data_t *>(user_data)->page_size, ++j) {
						if (vmr->get_index_alloc_mask(j))
							(*jt).vmr_index_tree.remove(&vmr->indices[j]);
					}
					if (!(*jt).vmr_index_tree.size()) {
						(*it).proc_related_vmrs.remove(static_cast<user_data_t *>(user_data)->pid);
						vt_g.release();
					}
				}
				if (!(*it).proc_related_vmrs.size()) {
					ki_reversal_mapping->remove(revmap_key);
					rvmr_g.release();
				}
			}
			return KF_CONTROL_FLOW_CONTINUE;
		},
		&user_data,
		KH_WALK_PGTAB_SKIP_UNMAPPED);

	vmr_a->size += vmr_b->size;

	ctxt->vmr_tree.remove(vmr_b);
	kfxx::destroy_at<mm_vmr_t>(vmr_b);
	kima_free(&*ctxt->kima_vmr_pool, vmr_b);

	return KM_RESULT_OK;
}

PBOS_NODISCARD PBOS_API km_result_t mm_split_mapped_area(
	mm_context_t *context,
	void *area_vaddr,
	void *split_point) {
	if (area_vaddr >= split_point) {
		dbg_println(__func__, "Trying to split a mapped area with area address >= split point");
		return KM_RESULT_INVALID_ARGS;
	}

	ki_mm_lock_vmr(context);
	kfxx::deferred lock_release([context]() noexcept {
		ki_mm_unlock_vmr(context);
	});

	mm_vmr_t *vmr;

	vmr = static_cast<mm_vmr_t *>(context->vmr_tree.find(area_vaddr));
	if (!vmr) {
		dbg_println(__func__, "VMR not found with address %p", area_vaddr);
		return KM_RESULT_INVALID_ARGS;
	}

	if (((char *)vmr->rb_value) + vmr->size <= split_point) {
		dbg_println(__func__, "Splitting area with split point %p which is beyond the area", split_point);
		return KM_RESULT_INVALID_ARGS;
	}

	mm_vmr_t *new_vmr = (mm_vmr_t *)kima_alloc(&*context->kima_vmr_pool, sizeof(mm_vmr_t), alignof(mm_vmr_t));
	if (!new_vmr)
		return KM_RESULT_NO_MEM;
	kfxx::construct_at<mm_vmr_t>(new_vmr);

	size_t latter_vmr_size = (((char *)vmr->rb_value) + vmr->size) - (char *)split_point;
	size_t split_point_off = (char *)split_point - (char *)vmr->rb_value;
	size_t page_size = kh_get_page_size();
	ps_proc_id_t pid = context->pcb->rb_value;

	struct user_data_t {
		mm_vmr_t *vmr;
		ps_proc_id_t pid;
		size_t page_size;
		km_result_t result;
	};
	user_data_t user_data;

	user_data = { new_vmr, pid, page_size, KM_RESULT_OK };
	kh_walk_pgtab(
		context,
		split_point,
		latter_vmr_size,
		[](void *vaddr, void *paddr, mm_page_access_t page_access, void *user_data) -> kf_control_flow_t {
			ki_reversal_map_t *m = nullptr;
			auto ud = static_cast<user_data_t *>(user_data);
			ps::read_semaphore_guard gm_g(ki_reversal_mapping_semaphore);
			uintptr_t revmap_key = ki_compute_reversal_mapping_key(static_cast<user_data_t *>(user_data)->page_size, reinterpret_cast<uintptr_t>(paddr));
			if (auto it = ki_reversal_mapping->find(revmap_key); it != ki_reversal_mapping->end()) {
				m = &*it;
			} else {
				if (!ki_reversal_mapping->insert(revmap_key, ki_reversal_map_t(kfxx::kernel_allocator()))) {
					ud->result = KM_RESULT_NO_MEM;
					return KF_CONTROL_FLOW_BREAK;
				}
				m = &ki_reversal_mapping->at(revmap_key);
			}

			ki_related_vmrs_entry_t *t = nullptr;
			ps_proc_id_t pid = ud->pid;
			ps::read_semaphore_guard rvmr_g(m->related_vmrs_semaphore);
			if (auto it = m->proc_related_vmrs.find(pid); it != m->proc_related_vmrs.end()) {
				t = &(*it);
			} else {
				if (!m->proc_related_vmrs.insert(pid, {})) {
					ud->result = KM_RESULT_NO_MEM;
					return KF_CONTROL_FLOW_BREAK;
				}
				t = &m->proc_related_vmrs.at(pid);
			}

			ps::read_semaphore_guard vt_g(t->vmr_index_tree_semaphore);
			kd_assert(ud->vmr->index_alloc_cur_index < ud->vmr->size / ud->page_size);
			t->vmr_index_tree.insert_unwrap(&ud->vmr->indices[ud->vmr->index_alloc_cur_index]);
			++ud->vmr->index_alloc_cur_index;

			return KF_CONTROL_FLOW_CONTINUE;
		},
		&user_data,
		KH_WALK_PGTAB_SKIP_UNMAPPED);

	auto free_walker = [](void *vaddr, void *paddr, mm_page_access_t page_access, void *user_data) -> kf_control_flow_t {
		if (!(page_access & MM_PAGE_MAPPED))
			return KF_CONTROL_FLOW_CONTINUE;

		mm_vmr_t *vmr = static_cast<user_data_t *>(user_data)->vmr;

		ps::read_semaphore_guard gm_g(ki_reversal_mapping_semaphore);
		uintptr_t revmap_key = ki_compute_reversal_mapping_key(static_cast<user_data_t *>(user_data)->page_size, reinterpret_cast<uintptr_t>(paddr));
		if (auto it = ki_reversal_mapping->find(revmap_key); it != ki_reversal_mapping->end()) {
			ps::read_semaphore_guard rvmr_g((*it).related_vmrs_semaphore);
			if (auto jt = (*it).proc_related_vmrs.find(static_cast<user_data_t *>(user_data)->pid); jt != (*it).proc_related_vmrs.end()) {
				ps::write_semaphore_guard vt_g((*jt).vmr_index_tree_semaphore);
				for (size_t k = 0, j = 0; k < vmr->size; k += static_cast<user_data_t *>(user_data)->page_size, ++j) {
					if (vmr->get_index_alloc_mask(j))
						(*jt).vmr_index_tree.remove(&vmr->indices[j]);
				}
				if (!(*jt).vmr_index_tree.size()) {
					(*it).proc_related_vmrs.remove(static_cast<user_data_t *>(user_data)->pid);
					vt_g.release();
				}
			}
			if (!(*it).proc_related_vmrs.size()) {
				ki_reversal_mapping->remove(revmap_key);
				rvmr_g.release();
			}
		}
		return KF_CONTROL_FLOW_CONTINUE;
	};

	if (KM_FAILED(user_data.result)) {
		km_result_t result = user_data.result;
		user_data.vmr = new_vmr;
		kh_walk_pgtab(
			context,
			split_point,
			latter_vmr_size,
			free_walker,
			&user_data,
			KH_WALK_PGTAB_SKIP_UNMAPPED);
		return result;
	} else {
		user_data = { vmr, pid, page_size, KM_RESULT_OK };
		kh_walk_pgtab(
			context,
			split_point,
			latter_vmr_size,
			free_walker,
			&user_data,
			KH_WALK_PGTAB_SKIP_UNMAPPED);
	}

	new_vmr->rb_value = split_point;
	new_vmr->size = latter_vmr_size;
	new_vmr->access = vmr->access;

	context->vmr_tree.insert_unwrap(new_vmr);

	vmr->size = ((char *)split_point) - (char *)area_vaddr;

	return KM_RESULT_OK;
}

PBOS_API km_result_t mm_set_page_access(
	mm_context_t *context,
	void *vaddr,
	size_t size,
	mm_page_access_t access) {
	if (!context)
		return KM_RESULT_INVALID_ARGS;
	if (!size)
		return KM_RESULT_INVALID_ARGS;
	void *aligned_vaddr = (void *)PGFLOOR(vaddr);
	size_t aligned_size = PGCEIL(size);
	char *vaddr_limit = (char *)aligned_vaddr + (size - 1);
	bool is_user_space = mm_is_user_space(aligned_vaddr);

	if (is_user_space !=
		mm_is_user_space(vaddr_limit)) {
		dbg_println(__func__, "Trying to set page access across kernel and user space");
		return KM_RESULT_INVALID_ARGS;
	}

	if (is_user_space) {
		ki_mm_lock_vmr(context);
		kfxx::deferred lock_release([context]() noexcept {
			ki_mm_unlock_vmr(context);
		});

		mm_vmr_t *vmr;

		vmr = static_cast<mm_vmr_t *>(context->vmr_tree.find(vaddr));

		if (!vmr) {
			dbg_println(__func__, "VMR not found with address %p", vaddr);
			return KM_RESULT_INVALID_ARGS;
		}

		if (vmr->size != size) {
			dbg_println(__func__, "Operation size does not match VMR's size");
			return KM_RESULT_INVALID_ARGS;
		}
		vmr->access = access;
		kh_set_page_access(context, vaddr, size, access);
	} else
		kh_set_page_access(context, vaddr, size, access);
	return KM_RESULT_OK;
}

PBOS_API void *mm_vmalloc(mm_context_t *context,
	const void *minaddr,
	const void *maxaddr,
	size_t size,
	mm_page_access_t access,
	mm_vmalloc_flags_t flags) {
	void *aligned_minaddr = (void *)PGFLOOR(minaddr);
	size_t aligned_size = PGCEIL(size);
	char *vaddr_limit = (char *)aligned_minaddr + (size - 1);
	bool is_user_space = mm_is_user_space(aligned_minaddr);

	if (is_user_space !=
		mm_is_user_space((void *)PGFLOOR(maxaddr))) {
		dbg_println(__func__, "Trying to allocate virtual memory across kernel and user space");
		return nullptr;
	}

	kfxx::scope_guard release_user_mmap_mutex_guard([context]() noexcept {
		context->vmr_mutex.unlock();
	});
	kfxx::scope_guard release_kernel_mmap_mutex_guard([]() noexcept {
		ki_kernel_mmap_mutex.unlock();
	});

	if (is_user_space) {
		release_kernel_mmap_mutex_guard.release();
		context->vmr_mutex.lock();
	} else {
		release_user_mmap_mutex_guard.release();
		ki_kernel_mmap_mutex.lock();
	}
	return kh_vmalloc(context, minaddr, maxaddr, size, access, flags);
}

PBOS_API void *mm_kvmalloc(mm_context_t *ctxt, size_t size, mm_page_access_t access, mm_vmalloc_flags_t flags) {
	return kh_kvmalloc(ctxt, size, access, flags);
}

PBOS_API void *mm_getmap(mm_context_t *ctxt, const void *vaddr, mm_page_access_t *page_access_out) {
	return kh_getmap(ctxt, vaddr, page_access_out);
}

PBOS_NODISCARD PBOS_API km_result_t mm_alloc_context(mm_context_t *cur_context, mm_context_t **new_context_out) {
	return kh_mm_alloc_context(cur_context, new_context_out);
}

void ki_mm_lock_vmr(mm_context_t *mm_context) {
	mm_context->vmr_mutex.lock();
}

void ki_mm_unlock_vmr(mm_context_t *mm_context) {
	mm_context->vmr_mutex.unlock();
}

void ki_mm_lock_area(mm_context_t *mm_context, mm_vmr_t *vmr) {
	vmr->mutex.lock();
}

void ki_mm_unlock_area(mm_context_t *mm_context, mm_vmr_t *vmr) {
	vmr->mutex.unlock();
}

PBOS_API km_result_t mm_probe_kernel_pages(mm_context_t *mm_context, void *ptr, size_t size, mm_page_access_t required_access) {
	const size_t page_size = mm_get_page_size();
	char *p = (char *)PGFLOOR((uintptr_t)ptr);

	// Overflow is an error.
	if (UINTPTR_MAX - (uintptr_t)p < PGCEIL(size))
		return KM_RESULT_INVALID_ARGS;

	char *limit = p + (PGCEIL(size) - 1);

	bool is_user_space = mm_is_user_space(p);
	if (is_user_space &&
		mm_is_user_space(limit))
		return KM_RESULT_INVALID_ARGS;

	mm_page_access_t pg_access;
	if (required_access & MM_PAGE_USER) {
		return KM_RESULT_ACCESS_VIOLATION;
	}

	// The mutex seems to be unneccessary, because we cannot lock pages under kernel mode.
	// ps::MutexGuard g(ki_kernel_mmap_mutex);

	for (size_t i = 0; i < PGCEIL(size); i += page_size) {
		mm_getmap(mm_context, p + i, &pg_access);
		if ((!(pg_access & MM_PAGE_MAPPED)))
			return KM_RESULT_ACCESS_VIOLATION;
		if ((required_access & MM_PAGE_USER) && !(pg_access & MM_PAGE_USER))
			return KM_RESULT_ACCESS_VIOLATION;
		if ((required_access & MM_PAGE_READ) && !(pg_access & MM_PAGE_READ))
			return KM_RESULT_ACCESS_VIOLATION;
		if ((required_access & MM_PAGE_WRITE) && !(pg_access & MM_PAGE_WRITE))
			return KM_RESULT_ACCESS_VIOLATION;
		if ((required_access & MM_PAGE_EXEC) && !(pg_access & MM_PAGE_EXEC))
			return KM_RESULT_ACCESS_VIOLATION;
	}

	return KM_RESULT_OK;
}

PBOS_API km_result_t mm_probe_and_lock_user_pages(mm_context_t *mm_context, void *ptr, size_t size, mm_page_access_t required_access) {
	// io::LocalIrqLock irq_lock;
	const size_t page_size = mm_get_page_size();
	char *p = (char *)PGFLOOR((uintptr_t)ptr);

	// Overflow is an error.
	if (UINTPTR_MAX - (uintptr_t)p < PGCEIL(size))
		return KM_RESULT_INVALID_ARGS;

	char *limit = p + (PGCEIL(size) - 1);

	bool is_user_space = mm_is_user_space(p);
	if (is_user_space !=
		mm_is_user_space(limit))
		return KM_RESULT_INVALID_ARGS;

	mm_page_access_t pg_access;

	if (!is_user_space) {
		return KM_RESULT_INVALID_ARGS;
	} else {
		ki_mm_lock_vmr(mm_context);
		kfxx::deferred lock_release([mm_context]() noexcept {
			ki_mm_unlock_vmr(mm_context);
		});

		mm_vmr_t *initial_vmr, *vmr, *last_vmr = nullptr, *vmr_end = static_cast<mm_vmr_t *>(mm_context->vmr_tree.find_max_lteq(limit));

		vmr = (initial_vmr = static_cast<mm_vmr_t *>(mm_context->vmr_tree.find_max_lteq(ptr)));

		if (!vmr)
			return KM_RESULT_INVALID_ARGS;

		kd_assert(vmr_end);

		kfxx::scope_guard restore_guard([mm_context, initial_vmr, &vmr]() noexcept {
			mm_vmr_t *i = vmr;

			while (i) {
				ki_mm_unlock_area(mm_context, i);

				i = static_cast<mm_vmr_t *>(ki_mm_vmr_tree_t::get_prev(vmr, initial_vmr));
			}
		});

		vmr = initial_vmr;

		while (vmr) {
			ki_mm_lock_area(mm_context, vmr);

			if (last_vmr) {
				if (((char *)last_vmr->rb_value) + last_vmr->size != vmr->rb_value)
					return KM_RESULT_ACCESS_VIOLATION;
			}
			last_vmr = vmr;

			pg_access = vmr->access;

			if ((required_access & MM_PAGE_READ) && !(pg_access & MM_PAGE_READ))
				return KM_RESULT_ACCESS_VIOLATION;
			if ((required_access & MM_PAGE_WRITE) && !(pg_access & MM_PAGE_WRITE))
				return KM_RESULT_ACCESS_VIOLATION;
			if ((required_access & MM_PAGE_EXEC) && !(pg_access & MM_PAGE_EXEC))
				return KM_RESULT_ACCESS_VIOLATION;

			vmr = static_cast<mm_vmr_t *>(ki_mm_vmr_tree_t::get_next(vmr, vmr_end));
		}

		if (((char *)last_vmr->rb_value) + last_vmr->size < limit)
			return KM_RESULT_ACCESS_VIOLATION;

		restore_guard.release();
	}

	return KM_RESULT_OK;
}

PBOS_API km_result_t mm_unlock_pages(mm_context_t *mm_context, void *ptr, size_t size) {
	const size_t page_size = mm_get_page_size();
	char *p = (char *)PGFLOOR((uintptr_t)ptr);

	// Overflow is an error.
	if (UINTPTR_MAX - (uintptr_t)p < PGCEIL(size))
		return KM_RESULT_INVALID_ARGS;

	char *limit = p + (PGCEIL(size) - 1);

	bool is_user_space = mm_is_user_space(p);
	if (is_user_space !=
		mm_is_user_space(limit))
		return KM_RESULT_INVALID_ARGS;

	mm_page_access_t pg_access;

	if (!is_user_space) {
		for (size_t i = 0; i < PGCEIL(size); i += page_size) {
			mm_getmap(mm_context, p + i, &pg_access);
			// TODO: Unlock the pages...
		}
	} else {
		ki_mm_lock_vmr(mm_context);
		kfxx::deferred lock_release([mm_context]() noexcept {
			ki_mm_unlock_vmr(mm_context);
		});

		mm_vmr_t *initial_vmr, *vmr, *last_vmr = nullptr, *vmr_end = static_cast<mm_vmr_t *>(mm_context->vmr_tree.find_max_lteq(limit));

		vmr = (initial_vmr = static_cast<mm_vmr_t *>(mm_context->vmr_tree.find_max_lteq(ptr)));

		if (!vmr)
			return KM_RESULT_INVALID_ARGS;

		kd_assert(vmr_end);

		vmr = initial_vmr;

		while (vmr) {
			if (last_vmr) {
				if (((char *)last_vmr->rb_value) + last_vmr->size != vmr->rb_value)
					km_panic("Unlocking pages with invalid area layout");
			}
			last_vmr = vmr;

			pg_access = vmr->access;

			ki_mm_unlock_area(mm_context, vmr);

			vmr = static_cast<mm_vmr_t *>(ki_mm_vmr_tree_t::get_next(vmr, vmr_end));
		}

		if (((char *)last_vmr->rb_value) + last_vmr->size < limit)
			km_panic("Unlocking pages with invalid area layout");
	}

	return KM_RESULT_OK;
}

PBOS_NODISCARD PBOS_API km_result_t mm_iommap(
	mm_context_t *context,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_page_access_t access,
	mm_iommap_flags_t flags) {
	km_result_t result;
	mmap_flags_t mmap_flags = MM_MMAP_NO_INC_RC;

	if ((result = mm_mmap(context, vaddr, paddr, size, access, mmap_flags)))
		return result;

	return KM_RESULT_OK;
}

PBOS_API void mm_uniommap(mm_context_t *context, void *vaddr, size_t size, mm_iommap_flags_t flags) {
	km_result_t result;
	mmap_flags_t mmap_flags = MM_MMAP_NO_INC_RC;

	km_unwrap_result(mm_munmap(context, vaddr, size, mmap_flags));
}

PBOS_API mm_context_t *mm_get_cur_context() {
	return mm_cur_contexts ? mm_cur_contexts[ps_get_cur_cpuid()] : mm_kernel_context;
}

void ki_mm_init() {
	kh_mm_init();

	ki_init_page_alloc_counter();

	ki_reversal_mapping = kfxx::radix_map_t<uintptr_t, ki_reversal_map_t, 3>{ kfxx::kernel_allocator() };

	ki_mm_init_global_allocator();
}
