#include <pbos/kd/logger.h>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/kh/mm/misc.hh>
#include <pbos/ki/km/symbol.hh>
#include <pbos/ki/ps/proc.hh>

mm_context_t **mm_cur_contexts = nullptr;
const ki_paging_config_t *ki_cur_paging_config;

ps::mutex_t ki_kernel_mmap_mutex;

_mm_context_t::_mm_context_t() {
	ki_init_kima_pool(&this->kima_common_pool);
	ki_init_kima_pool(&this->kima_vmr_pool);
}

PBOS_PURE PBOS_API size_t mm_get_page_size() {
	return kh_get_page_size();
}

PBOS_API km_result_t mm_mmap(mm_context_t *ctxt,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_pgaccess_t access,
	mmap_flags_t flags) {
	if (!ctxt)
		return KM_RESULT_INVALID_ARGS;
	if (!size)
		return KM_RESULT_INVALID_ARGS;

	void *aligned_vaddr = (void *)PGFLOOR(vaddr), *aligned_paddr = (void *)PGFLOOR(paddr);
	size_t aligned_size = PGCEIL(size);
	char *vaddr_limit = (char *)aligned_vaddr + (size - 1);
	bool is_user_space = mm_is_user_space(aligned_vaddr);

	// Overflow is an error.
	if (UINTPTR_MAX - (uintptr_t)aligned_vaddr < aligned_size) {
		kd_println(__func__, "Trying to unmap with address and size combination that will overflow");
		return KM_RESULT_INVALID_ARGS;
	}

	if (is_user_space !=
		mm_is_user_space(vaddr_limit)) {
		kd_println(__func__, "Trying to unmap across kernel and user space");
		return KM_RESULT_INVALID_ARGS;
	}

	if ((!is_user_space) && (access & MM_PAGE_USER)) {
		kd_println(__func__, "Trying to map kernel space with user accessible");
		return KM_RESULT_INVALID_ARGS;
	}

	kfxx::scope_guard remove_vmr_guard([ctxt, aligned_vaddr]() noexcept {
		if (auto node = ctxt->vmr_tree.find(aligned_vaddr))
			ctxt->vmr_tree.remove(node);
	});

	if (!(flags & MMAP_IGNORE_VMR)) {
		if (is_user_space) {
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

			mm_vmr_t *new_vmr = (mm_vmr_t *)kima_alloc(&ctxt->kima_vmr_pool, sizeof(mm_vmr_t), alignof(mm_vmr_t));
			if (!new_vmr)
				return KM_RESULT_NO_MEM;
			kfxx::construct_at<mm_vmr_t>(new_vmr);

			new_vmr->rb_value = vaddr;
			new_vmr->size = size;
			new_vmr->access = access;

			ctxt->vmr_tree.insert_unwrap(new_vmr);
		} else
			remove_vmr_guard.release();
	} else
		remove_vmr_guard.release();

	kfxx::scope_guard release_kernel_mmap_mutex_guard([]() noexcept {
		ki_kernel_mmap_mutex.unlock();
	});

	if (is_user_space)
		release_kernel_mmap_mutex_guard.release();
	else
		ki_kernel_mmap_mutex.lock();

	KM_RETURN_IF_FAILED(kh_mmap(ctxt, aligned_vaddr, aligned_paddr, aligned_size, access, flags));

	remove_vmr_guard.release();

	return KM_RESULT_OK;
}

PBOS_API km_result_t mm_unmmap(mm_context_t *ctxt, void *vaddr, size_t size, mmap_flags_t flags) {
	if (!ctxt)
		return KM_RESULT_INVALID_ARGS;

	void *aligned_vaddr = (void *)PGFLOOR(vaddr);
	size_t aligned_size = PGCEIL(size);
	char *vaddr_limit = (char *)aligned_vaddr + (size - 1);
	bool is_user_space = mm_is_user_space(aligned_vaddr);

	// Overflow is an error.
	if (UINTPTR_MAX - (uintptr_t)aligned_vaddr < aligned_size) {
		kd_println(__func__, "Trying to unmap with address and size combination that will overflow");
		return KM_RESULT_INVALID_ARGS;
	}

	if (is_user_space !=
		mm_is_user_space(vaddr_limit)) {
		kd_println(__func__, "Trying to unmap across kernel and user space");
		return KM_RESULT_INVALID_ARGS;
	}

	if (!(flags & MMAP_IGNORE_VMR)) {
		if (is_user_space) {
			ki_mm_lock_vmr(ctxt);
			kfxx::deferred lock_release([ctxt]() noexcept {
				ki_mm_unlock_vmr(ctxt);
			});

			mm_vmr_t *vmr;

			vmr = static_cast<mm_vmr_t *>(ctxt->vmr_tree.find(vaddr));

			if (!vmr)
				return KM_RESULT_INVALID_ARGS;

			if (size)
				return KM_RESULT_INVALID_ARGS;

			aligned_size = PGCEIL(vmr->size);

			ctxt->vmr_tree.remove(vmr);
			kfxx::destroy_at<mm_vmr_t>(vmr);
			kima_free(&ctxt->kima_vmr_pool, vmr);
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

	kh_unmmap(ctxt, aligned_vaddr, aligned_size, flags);
	return KM_RESULT_OK;
}

PBOS_NODISCARD PBOS_API km_result_t mm_merge_mapped_area(
	mm_context_t *context,
	void *vaddr_a,
	void *vaddr_b) {
	if (vaddr_a >= vaddr_b)
		return KM_RESULT_INVALID_ARGS;

	ki_mm_lock_vmr(context);
	kfxx::deferred lock_release([context]() noexcept {
		ki_mm_unlock_vmr(context);
	});

	mm_vmr_t *vmr_a, *vmr_b;

	vmr_a = static_cast<mm_vmr_t *>(context->vmr_tree.find(vaddr_a));
	if (!vmr_a) {
		kd_println(__func__, "VMR not found with address %p", vaddr_a);
		return KM_RESULT_INVALID_ARGS;
	}

	vmr_b = static_cast<mm_vmr_t *>(context->vmr_tree.find(vaddr_b));
	if (!vmr_b) {
		kd_println(__func__, "VMR not found with address %p", vaddr_b);
		return KM_RESULT_INVALID_ARGS;
	}

	if (((char *)vmr_a->rb_value) + vmr_a->size != vmr_b->rb_value) {
		kd_println(__func__, "VMR at %p is not neighbor of VMR %p", vaddr_a, vaddr_b);
		return KM_RESULT_INVALID_ARGS;
	}

	vmr_a->size += vmr_b->size;

	context->vmr_tree.remove(vmr_b);
	kfxx::destroy_at<mm_vmr_t>(vmr_b);
	kima_free(&context->kima_vmr_pool, vmr_b);

	return KM_RESULT_OK;
}

PBOS_NODISCARD PBOS_API km_result_t mm_split_mapped_area(
	mm_context_t *context,
	void *area_vaddr,
	void *split_point) {
	if (area_vaddr >= split_point) {
		kd_println(__func__, "Trying to split a mapped area with area address >= split point");
		return KM_RESULT_INVALID_ARGS;
	}

	ki_mm_lock_vmr(context);
	kfxx::deferred lock_release([context]() noexcept {
		ki_mm_unlock_vmr(context);
	});

	mm_vmr_t *vmr;

	vmr = static_cast<mm_vmr_t *>(context->vmr_tree.find(area_vaddr));
	if (!vmr) {
		kd_println(__func__, "VMR not found with address %p", area_vaddr);
		return KM_RESULT_INVALID_ARGS;
	}

	if (((char *)vmr->rb_value) + vmr->size <= split_point) {
		kd_println(__func__, "Splitting area with split point %p which is beyond the area", split_point);
		return KM_RESULT_INVALID_ARGS;
	}

	mm_vmr_t *new_vmr = (mm_vmr_t *)kima_alloc(&context->kima_vmr_pool, sizeof(mm_vmr_t), alignof(mm_vmr_t));
	if (!new_vmr)
		return KM_RESULT_NO_MEM;
	kfxx::construct_at<mm_vmr_t>(new_vmr);

	new_vmr->rb_value = split_point;
	new_vmr->size = (((char *)vmr->rb_value) + vmr->size) - (char *)split_point;
	new_vmr->access = vmr->access;

	context->vmr_tree.insert_unwrap(new_vmr);

	vmr->size = ((char *)split_point) - (char *)area_vaddr;

	return KM_RESULT_OK;
}

PBOS_API km_result_t mm_set_page_access(
	mm_context_t *context,
	void *vaddr,
	size_t size,
	mm_pgaccess_t access) {
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
		kd_println(__func__, "Trying to set page access across kernel and user space");
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
			kd_println(__func__, "VMR not found with address %p", vaddr);
			return KM_RESULT_INVALID_ARGS;
		}

		if (vmr->size != size) {
			kd_println(__func__, "Operation size does not match VMR's size");
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
	mm_pgaccess_t access,
	mm_vmalloc_flags_t flags) {
	void *aligned_minaddr = (void *)PGFLOOR(minaddr);
	size_t aligned_size = PGCEIL(size);
	char *vaddr_limit = (char *)aligned_minaddr + (size - 1);
	bool is_user_space = mm_is_user_space(aligned_minaddr);

	if (is_user_space !=
		mm_is_user_space((void *)PGFLOOR(maxaddr))) {
		kd_println(__func__, "Trying to allocate virtual memory across kernel and user space");
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

PBOS_API void *mm_kvmalloc(mm_context_t *ctxt, size_t size, mm_pgaccess_t access, mm_vmalloc_flags_t flags) {
	return kh_kvmalloc(ctxt, size, access, flags);
}

PBOS_API void *mm_getmap(mm_context_t *ctxt, const void *vaddr, mm_pgaccess_t *pgaccess_out) {
	return kh_getmap(ctxt, vaddr, pgaccess_out);
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

PBOS_API km_result_t mm_probe_kernel_pages(mm_context_t *mm_context, void *ptr, size_t size, mm_pgaccess_t required_access) {
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

	mm_pgaccess_t pg_access;
	if (required_access & MM_PAGE_USER) {
		return KM_RESULT_ACCESS_VIOLATION;
	}

	// The mutex seems to be unneccessary, because we cannot lock pages under kernel mode.
	// ps::MutexGuard g(ki_kernel_mmap_mutex.c_mutex());

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

PBOS_API km_result_t mm_probe_and_lock_user_pages(mm_context_t *mm_context, void *ptr, size_t size, mm_pgaccess_t required_access) {
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

	mm_pgaccess_t pg_access;

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

	mm_pgaccess_t pg_access;

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

PBOS_API mm_context_t *mm_get_cur_context() {
	return mm_cur_contexts ? mm_cur_contexts[ps_get_cur_cpuid()] : mm_kernel_context;
}
