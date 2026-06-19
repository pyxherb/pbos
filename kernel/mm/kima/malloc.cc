#include <pbos/kd/logger.h>
#include <string.h>
#include <kernel/generated/config.hh>
#include <pbos/hal/irq.hh>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/ki/kasan/impl.hh>
#include <pbos/ki/km/symbol.hh>
#include <pbos/ki/mm/kima.hh>

void *kima_alloc(kima_pool_t *pool, size_t size, size_t alignment) {
	kd_dbgcheck(pool->_initialized == true, "The pool must be initialized");
	kd_dbgcheck((alignment & (alignment - 1)) == 0, "The alignment must be power of 2");
	kd_dbgcheck(size, "The size for mm_kalloc must not be 0");

	ps::mutex_guard g(pool->mutex);

	size_t size_log2 = kima_log2(size), alignment_log2 = kima_log2(alignment);
	if ((size_log2 > KIMA_SMALL_BLOCK_MAX_ORDER) || (alignment > KIMA_SMALL_BLOCK_MAX_ORDER)) {
		char *continuous_area_base = nullptr;

		const size_t aligned_page_size = kfxx::ceil_align_to(size, pool->page_size);

		for (auto it = pool->vpgdesc_query_tree.begin(); it != pool->vpgdesc_query_tree.end();) {
			kima_vpgdesc_t *cur_desc = static_cast<kima_vpgdesc_t *>(it.node);

			if (cur_desc->rb_value < continuous_area_base) {
				++it;
				continue;
			}

			if (cur_desc->recommended_alloc_off >= pool->page_size) {
				++it;
				continue;
			}

			for (size_t j = pool->page_size;
				j < aligned_page_size;
				j += pool->page_size) {
				auto old_it = it;
				if (++it == pool->vpgdesc_query_tree.end())
					goto alloc_new_page;
				if ((char *)old_it.node->rb_value + pool->page_size < it.node->rb_value) {
					continuous_area_base = (char *)it.node->rb_value;
					it = decltype(pool->vpgdesc_query_tree)::iterator(
						pool->vpgdesc_query_tree.find_max_lteq(continuous_area_base),
						&pool->vpgdesc_query_tree,
						kfxx::iterator_direction::Forward);
					goto noncontinuous;
				}
			}

			{
				void *const limit = ((char *)cur_desc->rb_value) + aligned_page_size;

				char *cur_base = (char *)kfxx::ceil_align_to((uintptr_t)cur_desc->rb_value + cur_desc->recommended_alloc_off, alignment);
				auto i = decltype(pool->ublk_query_tree)::iterator(
						 pool->ublk_query_tree.find_max_lteq(((char *)cur_base) + size - 1),
						 &pool->ublk_query_tree,
						 kfxx::iterator_direction::Forward),
					 last_i = i;
				while (cur_base + size < limit) {
					auto node = static_cast<kima_ublk_t *>(i.node);
					if (i == pool->ublk_query_tree.end())
						goto alloc_new_page;
					char *blk_limit = ((char *)node->rb_value + node->size);
					if (((char *)node->rb_value <= cur_base) &&
						(blk_limit > cur_base)) {
						cur_base = blk_limit;
						++i;
						last_i = i;
					} else {
						auto verify = pool->ublk_query_tree.find_max_lteq(((char *)cur_base) + size - 1);

						if ((char *)verify->rb_value + static_cast<kima_ublk_t *>(verify)->size > cur_base) {
							i = decltype(pool->ublk_query_tree)::iterator(
								verify,
								&pool->ublk_query_tree,
								kfxx::iterator_direction::Forward)
									.next();
							cur_base = (char *)verify->rb_value + static_cast<kima_ublk_t *>(verify)->size;
							continue;
						}

						kima_ublk_t *ublk = kima_alloc_ublk(pool, cur_base, size);
						if (!ublk)
							return nullptr;

						bool update_recommended_alloc_off = ((uintptr_t)node->rb_value - (uintptr_t)last_i.node->rb_value) >= (size >> 1);

						uintptr_t bottom = kfxx::floor_align_to((uintptr_t)cur_base, pool->page_size);
						for (uintptr_t j = bottom, k = 0;
							j < kfxx::ceil_align_to((uintptr_t)cur_base + size, pool->page_size);
							j += pool->page_size, k += pool->page_size) {
							kima_vpgdesc_t *vpgdesc = kima_lookup_vpgdesc(pool, (void *)j);

							if (update_recommended_alloc_off && (!k)) {
								vpgdesc->recommended_alloc_off = (size_t)last_i.node->rb_value + static_cast<kima_ublk_t *>(last_i.node)->size;
							} else if (k + pool->page_size >= size) {
								vpgdesc->recommended_alloc_off = (size_t)cur_base - j;
							} else {
								vpgdesc->recommended_alloc_off = pool->page_size;
							}

							++vpgdesc->ref_count;
						}

#if KI_ENABLE_KASAN
						ki_kasan_unpoison_addr(cur_base, size);
#endif

						return cur_base;
					}
				}
			}

			++it;
		noncontinuous:;
		}

	alloc_new_page:
		char *new_free_pg = (char *)kima_vpgalloc(pool, kfxx::ceil_align_to(size + alignment, pool->page_size)),
			 *aligned_new_free_pg = (char *)kfxx::ceil_align_to((uintptr_t)new_free_pg, alignment);

		if (!new_free_pg)
			return nullptr;

		size_t i = 0;
		kfxx::scope_guard release_vpgdesc_sg([pool, new_free_pg, &i]() noexcept {
			for (size_t j = 0; j < i; j += pool->page_size) {
				kima_free_vpgdesc(pool, kima_lookup_vpgdesc(pool, ((char *)new_free_pg) + j));
			}
		});

		for (; i < size; i += pool->page_size) {
			kima_vpgdesc_t *vpgdesc = kima_alloc_vpgdesc(pool, ((char *)new_free_pg) + i);

			if (!vpgdesc)
				return nullptr;

			++vpgdesc->ref_count;
		}

		kima_ublk_t *ublk = kima_alloc_ublk(pool, new_free_pg, size);
		if (!ublk)
			return nullptr;

		release_vpgdesc_sg.release();

		void *addr = (void *)kfxx::ceil_align_to((uintptr_t)new_free_pg, alignment);

#if KI_ENABLE_KASAN
		ki_kasan_unpoison_addr(addr, size);
#endif

		return addr;
	} else {
		kima_small_block_desc_t *desc = kima_alloc_small_block_desc(pool, PBOS_MAX(PBOS_MAX(size_log2, alignment_log2), KIMA_SMALL_BLOCK_MIN_ORDER));

		desc->allocated_size = size;

		memset(desc->ptr, 0, size);

#if KI_ENABLE_KASAN
		ki_kasan_unpoison_addr(desc->ptr, size);
#endif

		return desc->ptr;
	}
	PBOS_UNREACHABLE();
}

PBOS_NODISCARD void *kima_realloc(kima_pool_t *pool, void *old_ptr, size_t size, size_t alignment) {
	kd_dbgcheck(pool->_initialized == true, "The pool must be initialized");
	kd_dbgcheck(size, "The new block size must not be 0");
	ps::mutex_guard g(pool->mutex);

	kima_ublk_t *old_ublk = static_cast<kima_ublk_t *>(pool->ublk_query_tree.find(old_ptr));

	size_t size_log2 = kima_log2(size), alignment_log2 = kima_log2(alignment);
	if (old_ublk) {
		if ((size_log2 > KIMA_SMALL_BLOCK_MAX_ORDER) || (alignment > KIMA_SMALL_BLOCK_MAX_ORDER)) {
			char *continuous_area_base = nullptr;

			for (auto it = pool->vpgdesc_query_tree.begin(); it != pool->vpgdesc_query_tree.end();) {
				kima_vpgdesc_t *cur_desc = static_cast<kima_vpgdesc_t *>(it.node);

				if (cur_desc->rb_value < continuous_area_base) {
					++it;
					continue;
				}

				for (size_t j = 0;
					j < kfxx::ceil_align_to(size, pool->page_size);
					j += pool->page_size) {
					// Look up for a continuous existing allocated virtual page area for allocation.
					if (!kima_lookup_vpgdesc(pool, ((char *)cur_desc->rb_value) + j)) {
						continuous_area_base = ((char *)cur_desc->rb_value) + j;
						it = kfxx::rbtree_t<void *>::iterator(pool->vpgdesc_query_tree.find_max_lteq(continuous_area_base), &pool->vpgdesc_query_tree, kfxx::iterator_direction::Forward);
						goto noncontinuous;
					}
				}

				{
					void *const limit = ((char *)cur_desc->rb_value) + (kfxx::ceil_align_to(size, pool->page_size) - size);

					for (char *cur_base = (char *)cur_desc->rb_value;
						cur_base <= limit;) {
						if (size_t aligned_diff = ((uintptr_t)cur_base) % alignment; aligned_diff) {
							cur_base += alignment - aligned_diff;
						}

						kima_ublk_t *nearest_ublk;
						if ((nearest_ublk = kima_lookup_nearest_ublk(pool, cur_base))) {
							if (nearest_ublk != old_ublk) {
								if (PBOS_ISOVERLAPPED((char *)cur_base, size, (char *)nearest_ublk->rb_value, nearest_ublk->size)) {
									cur_base = ((char *)nearest_ublk->rb_value) + nearest_ublk->size;
									continue;
								}
							}
						}
						if ((nearest_ublk = kima_lookup_nearest_ublk(pool, ((char *)cur_base) + size - 1))) {
							if (nearest_ublk != old_ublk) {
								if (PBOS_ISOVERLAPPED((char *)cur_base, size, (char *)nearest_ublk->rb_value, nearest_ublk->size)) {
									cur_base = ((char *)nearest_ublk->rb_value) + nearest_ublk->size;
									continue;
								}
							}
						}

						pool->ublk_query_tree.remove(old_ublk);

						for (size_t j = 0;
							j < kfxx::ceil_align_to(size, pool->page_size);
							j += pool->page_size) {
							kima_vpgdesc_t *vpgdesc = kima_lookup_vpgdesc(pool, ((char *)cur_desc->rb_value) + j);

							if (!vpgdesc)
								km_panic("BUG: vpgdesc == nullptr, please report this bug to the developer");

							++vpgdesc->ref_count;
						}

						memmove(cur_base, old_ptr, old_ublk->size);

						// Because there is always a enough virtual memory area as we judged before,
						// we just need to release the unused old virtual pages.
						// And because we had increased the reference counter of the new used pages,
						// the old unused pages will be freed as they didn't get the reference counter
						// increased.
						for (uintptr_t i = kfxx::floor_align_to((uintptr_t)old_ptr, pool->page_size);
							i < kfxx::ceil_align_to(((uintptr_t)old_ptr) + old_ublk->size, pool->page_size);
							i += pool->page_size) {
							kima_vpgdesc_t *vpgdesc = kima_lookup_vpgdesc(pool, (void *)i);

							kd_assert(vpgdesc);

							if (!(--vpgdesc->ref_count))
								kima_free_vpgdesc(pool, vpgdesc);
						}

						old_ublk->rb_value = cur_base;	// rb_value is the base block address.
						old_ublk->size = size;

						pool->ublk_query_tree.insert_unwrap(old_ublk);

						return cur_base;
					}
				}

				++it;
			noncontinuous:;
			}
			// Allocate new pages if there is no suitable continuous virtual memory area.
			char *new_free_pg = (char *)kima_vpgalloc(pool, kfxx::ceil_align_to(size + alignment, pool->page_size));

			if (size_t aligned_diff = ((uintptr_t)new_free_pg) % alignment; aligned_diff) {
				new_free_pg += alignment - aligned_diff;
			}

			if (!new_free_pg)
				return nullptr;

			size_t i = 0;
			kfxx::scope_guard release_vpgdesc_sg([pool, new_free_pg, &i]() noexcept {
				for (size_t j = 0; j < i; j += pool->page_size) {
					kima_free_vpgdesc(pool, kima_lookup_vpgdesc(pool, ((char *)new_free_pg) + j));
				}
			});

			for (; i < size; i += pool->page_size) {
				kima_vpgdesc_t *vpgdesc = kima_alloc_vpgdesc(pool, ((char *)new_free_pg) + i);

				if (!vpgdesc)
					return nullptr;

				++vpgdesc->ref_count;
			}

#if KI_ENABLE_KASAN
			ki_kasan_poison_addr(old_ptr, old_ublk->size, KASAN_SHADOW_VALUE_FREE);
#endif

			kima_free_ublk(pool, old_ublk);

			kima_ublk_t *ublk = kima_alloc_ublk(pool, new_free_pg, size);
			if (!ublk)
				return nullptr;

#if KI_ENABLE_KASAN
			ki_kasan_unpoison_addr(new_free_pg, size);
#endif

			release_vpgdesc_sg.release();

			return new_free_pg;
		} else {
			void *ptr = kima_alloc(pool, size, alignment);

			if (!ptr)
				return nullptr;

			memcpy(ptr, old_ptr, PBOS_MIN(size, old_ublk->size));

			kima_free(pool, old_ptr);

			return ptr;
		}
		PBOS_UNREACHABLE();
	} else {
		void *ptr = kima_alloc(pool, size, alignment);

		if (!ptr)
			return nullptr;

		memcpy(ptr, old_ptr, PBOS_MIN(size, old_ublk->size));

		kima_free(pool, old_ptr);

		return ptr;
	}
	PBOS_UNREACHABLE();
}

PBOS_NODISCARD void *kima_realloc_in_place(kima_pool_t *pool, void *old_ptr, size_t size) {
	kd_dbgcheck(pool->_initialized == true, "The pool must be initialized");
	kd_dbgcheck(size, "The new block size must not be 0");
	ps::mutex_guard g(pool->mutex);
	char *continuous_area_base = nullptr;

	kima_ublk_t *old_ublk = static_cast<kima_ublk_t *>(pool->ublk_query_tree.find(old_ptr));

	if (old_ublk) {
		kima_vpgdesc_t *cur_desc = static_cast<kima_vpgdesc_t *>(pool->vpgdesc_query_tree.find(old_ptr));

		for (size_t j = 0;
			j < kfxx::ceil_align_to(size, pool->page_size);
			j += pool->page_size) {
			// Look up for a continuous existing allocated virtual page area for allocation.
			if (!kima_lookup_vpgdesc(pool, ((char *)cur_desc->rb_value) + j)) {
				continuous_area_base = ((char *)cur_desc->rb_value) + j;
				return nullptr;
			}
		}

		{
			void *const limit = ((char *)cur_desc->rb_value) + (kfxx::ceil_align_to(size, pool->page_size) - size);

			kima_ublk_t *nearest_ublk;
			if ((nearest_ublk = kima_lookup_nearest_ublk(pool, ((char *)old_ptr) + size - 1))) {
				if (nearest_ublk != old_ublk) {
					if (PBOS_ISOVERLAPPED((char *)old_ptr, size, (char *)nearest_ublk->rb_value, nearest_ublk->size)) {
						return nullptr;
					}
				}
			}

			pool->ublk_query_tree.remove(old_ublk);

			for (size_t j = 0;
				j < kfxx::ceil_align_to(size, pool->page_size);
				j += pool->page_size) {
				kima_vpgdesc_t *vpgdesc = kima_lookup_vpgdesc(pool, ((char *)cur_desc->rb_value) + j);

				if (!vpgdesc)
					km_panic("BUG: vpgdesc == nullptr, please report this bug to the developer");

				++vpgdesc->ref_count;
			}

			// Release unused old pages if the new size is smaller than the old size.
			for (size_t i = kfxx::ceil_align_to(old_ublk->size, pool->page_size);
				i > kfxx::ceil_align_to(size, pool->page_size);
				++i) {
				kima_vpgdesc_t *vpgdesc = kima_lookup_vpgdesc(pool, ((char *)old_ptr) + i);

				kd_assert(vpgdesc);

				if (!(--vpgdesc->ref_count))
					kima_free_vpgdesc(pool, vpgdesc);
			}

			old_ublk->size = size;

			pool->ublk_query_tree.insert_unwrap(old_ublk);

			return old_ptr;
		}

		return nullptr;
	} else {
		uintptr_t floor_addr = kfxx::floor_align_to((uintptr_t)old_ptr, pool->page_size);
		uintptr_t ceil_addr = floor_addr + pool->page_size;

		kima_small_block_page_desc_t *page_desc = (kima_small_block_page_desc_t *)(ceil_addr - sizeof(kima_small_block_desc_t));

		if (((uintptr_t)old_ptr) % KIMA_ORDERED_BLOCK_SIZE(page_desc->order))
			km_panic("Reallocating an invalid block, with %p", old_ptr);

		size_t log2_size = kima_log2(size);

		if (log2_size > page_desc->order)
			return nullptr;

		// TODO: Change the size...

		// DEBUG begin
		size_t off = ((uintptr_t)old_ptr) - floor_addr;

		size_t block_index = kima_calc_small_block_index(pool, page_desc->order, off);
		size_t index = page_desc->order - KIMA_SMALL_BLOCK_MIN_ORDER;

		kima_small_block_desc_t *desc = (kima_small_block_desc_t *)(floor_addr + pool->small_block_page_info[index].block_descs_off + sizeof(kima_small_block_desc_t) * block_index);

		kd_dbgcheck(desc->ptr == old_ptr, "The pointer to be freed %p does not match the descriptor's %p", old_ptr, desc->ptr);
		kd_dbgcheck(!desc->is_free, "The block at %p is not allocated, perhaps the descriptor is damaged by heap buffer overflows?", old_ptr);
		// DEBUG end

#if KI_ENABLE_KASAN
		ki_kasan_poison_addr(old_ptr, desc->allocated_size, KASAN_SHADOW_VALUE_FREE);
#endif

		desc->allocated_size = size;

#if KI_ENABLE_KASAN
		ki_kasan_unpoison_addr(old_ptr, size);
#endif

		return old_ptr;
	}
}

void kima_free(kima_pool_t *pool, void *ptr) {
	kd_dbgcheck(pool->_initialized == true, "The pool must be initialized");
	// io::LocalIrqLock irq_lock;

	ps::mutex_guard g(pool->mutex);

	kima_ublk_t *ublk = kima_lookup_ublk(pool, ptr);

	if (ublk) {
		for (uintptr_t i = kfxx::floor_align_to((uintptr_t)ublk->rb_value, pool->page_size),
					   j = 0;
			i < kfxx::ceil_align_to(((uintptr_t)ublk->rb_value) + ublk->size, pool->page_size);
			i += pool->page_size, j += pool->page_size) {
			kima_vpgdesc_t *vpgdesc = kima_lookup_vpgdesc(pool, (void *)i);

			kd_assert(vpgdesc);

			if (!j) {
				vpgdesc->recommended_alloc_off = (uintptr_t)ublk->rb_value - i;
			} else if (j + pool->page_size >= ublk->size) {
				vpgdesc->recommended_alloc_off = 0;
			} else {
				vpgdesc->recommended_alloc_off = 0;
			}

			if (!(--vpgdesc->ref_count)) {
				kima_free_vpgdesc(pool, vpgdesc);
			}
		}

#if KI_ENABLE_KASAN
		ki_kasan_poison_addr(ptr, ublk->size, KASAN_SHADOW_VALUE_FREE);
#endif

		kima_free_ublk(pool, ublk);
	} else {
		uintptr_t floor_addr = kfxx::floor_align_to((uintptr_t)ptr, pool->page_size);
		uintptr_t ceil_addr = floor_addr + pool->page_size;

		kima_small_block_page_desc_t *page_desc = (kima_small_block_page_desc_t *)(ceil_addr - sizeof(kima_small_block_desc_t));

		if (((uintptr_t)ptr) % KIMA_ORDERED_BLOCK_SIZE(page_desc->order))
			km_panic("Freeing an invalid block, with %p", ptr);

		size_t off = ((uintptr_t)ptr) - floor_addr;

		size_t block_index = kima_calc_small_block_index(pool, page_desc->order, off);
		size_t index = page_desc->order - KIMA_SMALL_BLOCK_MIN_ORDER;

		kima_small_block_desc_t *desc = (kima_small_block_desc_t *)(floor_addr + pool->small_block_page_info[index].block_descs_off + sizeof(kima_small_block_desc_t) * block_index);

		kd_dbgcheck(desc->ptr == ptr, "The pointer to be freed %p does not match the descriptor's %p", ptr, desc->ptr);
		kd_dbgcheck(!desc->is_free, "The block at %p is not allocated, perhaps the descriptor is damaged by heap buffer overflows?", ptr);

#if KI_ENABLE_KASAN
		ki_kasan_poison_addr(ptr, desc->allocated_size, KASAN_SHADOW_VALUE_FREE);
#endif

		kima_free_small_block_desc(pool, page_desc, desc);
	}
}
