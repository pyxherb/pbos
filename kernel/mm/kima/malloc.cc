#include <string.h>
#include <pbos/hal/irq.hh>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/ki/km/symbol.hh>
#include <pbos/ki/mm/kima.hh>

void *kima_alloc(kima_pool_t *pool, size_t size, size_t alignment) {
	kd_assert(pool->_initialized);
	// io::LocalIrqLock irq_lock;
	ps::mutex_guard g(pool->mutex.c_mutex());

	kd_dbgcheck(size, "The size for mm_kalloc must not be 0");
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
			if (!kima_lookup_vpgdesc(pool, ((char *)cur_desc->rb_value) + j)) {
				continuous_area_base = ((char *)cur_desc->rb_value) + j;
				it = kfxx::rbtree_t<void *>::iterator(pool->vpgdesc_query_tree.find_max_lteq(continuous_area_base), &pool->vpgdesc_query_tree, kfxx::iteratorDirection::Forward);
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
					if (PBOS_ISOVERLAPPED((char *)cur_base, size, (char *)nearest_ublk->rb_value, nearest_ublk->size)) {
						cur_base = ((char *)nearest_ublk->rb_value) + nearest_ublk->size;
						continue;
					}
				}
				if ((nearest_ublk = kima_lookup_nearest_ublk(pool, ((char *)cur_base) + size - 1))) {
					if (PBOS_ISOVERLAPPED((char *)cur_base, size, (char *)nearest_ublk->rb_value, nearest_ublk->size)) {
						cur_base = ((char *)nearest_ublk->rb_value) + nearest_ublk->size;
						continue;
					}
				}

				kima_ublk_t *ublk = kima_alloc_ublk(pool, cur_base, size);
				if (!ublk)
					return nullptr;

				for (size_t j = 0;
					j < kfxx::ceil_align_to(size, pool->page_size);
					j += pool->page_size) {
					kima_vpgdesc_t *vpgdesc = kima_lookup_vpgdesc(pool, ((char *)cur_desc->rb_value) + j);

					if (!vpgdesc)
						km_panic("BUG: vpgdesc == nullptr, please report this bug to the developer");

					++vpgdesc->ref_count;
				}

				return cur_base;
			}
		}

		++it;
	noncontinuous:;
	}

	char *new_free_pg = (char *)kima_vpgalloc(pool, NULL, kfxx::ceil_align_to(size + alignment, pool->page_size));

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
		pool->num_allocated_pages -= i;
	});

	for (; i < size; i += pool->page_size) {
		kima_vpgdesc_t *vpgdesc = kima_alloc_vpgdesc(pool, ((char *)new_free_pg) + i);

		if (!vpgdesc)
			return nullptr;

		++vpgdesc->ref_count;
	}

	pool->num_allocated_pages += kfxx::ceil_align_to(size, pool->page_size) / pool->page_size;

	kima_ublk_t *ublk = kima_alloc_ublk(pool, new_free_pg, size);
	if (!ublk)
		return nullptr;

	release_vpgdesc_sg.release();

	return new_free_pg;
}

PBOS_NODISCARD void *kima_realloc(kima_pool_t *pool, void *old_ptr, size_t size, size_t alignment) {
	kd_assert(pool->_initialized);
	kd_assert(size);
	ps::mutex_guard g(pool->mutex.c_mutex());
	char *continuous_area_base = nullptr;

	kima_ublk_t *old_ublk = static_cast<kima_ublk_t *>(pool->ublk_query_tree.find(old_ptr));
	kd_assert(old_ublk);

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
				it = kfxx::rbtree_t<void *>::iterator(pool->vpgdesc_query_tree.find_max_lteq(continuous_area_base), &pool->vpgdesc_query_tree, kfxx::iteratorDirection::Forward);
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
	char *new_free_pg = (char *)kima_vpgalloc(pool, NULL, kfxx::ceil_align_to(size + alignment, pool->page_size));

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
		pool->num_allocated_pages -= i;
	});

	for (; i < pool->page_size; ++i) {
		kima_vpgdesc_t *vpgdesc = kima_alloc_vpgdesc(pool, ((char *)new_free_pg) + i);

		if (!vpgdesc)
			return nullptr;

		++vpgdesc->ref_count;
	}

	pool->num_allocated_pages += kfxx::ceil_align_to(size, pool->page_size) / pool->page_size;

	kima_free_ublk(pool, old_ublk);

	kima_ublk_t *ublk = kima_alloc_ublk(pool, new_free_pg, size);
	if (!ublk)
		return nullptr;

	release_vpgdesc_sg.release();

	return new_free_pg;
}

PBOS_NODISCARD void *kima_realloc_in_place(kima_pool_t *pool, void *old_ptr, size_t size, size_t alignment) {
	kd_assert(pool->_initialized);
	kd_assert(size);
	ps::mutex_guard g(pool->mutex.c_mutex());
	char *continuous_area_base = nullptr;

	kima_ublk_t *old_ublk = static_cast<kima_ublk_t *>(pool->ublk_query_tree.find(old_ptr));
	kd_assert(old_ublk);

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

		if (size_t aligned_diff = ((uintptr_t)old_ptr) % alignment; aligned_diff)
			return nullptr;

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
}

void kima_free(kima_pool_t *pool, void *ptr) {
	kd_assert(pool->_initialized);
	// io::LocalIrqLock irq_lock;

	ps::mutex_guard g(pool->mutex.c_mutex());

	kima_ublk_t *ublk = kima_lookup_ublk(pool, ptr);
	kd_assert(ublk);
	for (uintptr_t i = kfxx::floor_align_to((uintptr_t)ublk->rb_value, pool->page_size);
		i < kfxx::ceil_align_to(((uintptr_t)ublk->rb_value) + ublk->size, pool->page_size);
		i += pool->page_size) {
		kima_vpgdesc_t *vpgdesc = kima_lookup_vpgdesc(pool, (void *)i);

		kd_assert(vpgdesc);

		if (!(--vpgdesc->ref_count))
			kima_free_vpgdesc(pool, vpgdesc);
	}

	kima_free_ublk(pool, ublk);
}
