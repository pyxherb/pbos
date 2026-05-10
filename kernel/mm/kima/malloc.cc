#include <pbos/ki/mm/kima.hh>
#include <string.h>
#include <pbos/hal/irq.hh>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/ki/km/symbol.hh>

void *kima_alloc(kima_pool_t *pool, size_t size, size_t alignment) {
	// io::irq_disable_lock irq_lock;

	kd_dbgcheck(size, "The size for mm_kalloc must not be 0");
	char *continuous_area_base = nullptr;

	for (auto it = pool->vpgdesc_query_tree.begin(); it != pool->vpgdesc_query_tree.end();) {
		kima_vpgdesc_t *cur_desc = static_cast<kima_vpgdesc_t *>(it.node);

		if (cur_desc->rb_value < continuous_area_base) {
			++it;
			continue;
		}

		for (size_t j = 0;
			j < PGCEIL(size);
			j += PAGESIZE) {
			if (!kima_lookup_vpgdesc(pool, ((char *)cur_desc->rb_value) + j)) {
				continuous_area_base = ((char *)cur_desc->rb_value) + j;
				it = kfxx::rbtree_t<void *>::iterator(pool->vpgdesc_query_tree.find_max_lteq(continuous_area_base), &pool->vpgdesc_query_tree, kfxx::iterator_direction::Forward);
				goto noncontinuous;
			}
		}

		{
			void *const limit = ((char *)cur_desc->rb_value) + (PGCEIL(size) - size);

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
					return NULL;

				for (size_t j = 0;
					j < PGCEIL(size);
					j += PAGESIZE) {
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

	char *new_free_pg = (char *)kima_vpgalloc(NULL, PGCEIL(size + alignment));

	if (size_t aligned_diff = ((uintptr_t)new_free_pg) % alignment; aligned_diff) {
		new_free_pg += alignment - aligned_diff;
	}

	if (!new_free_pg)
		return NULL;

	size_t i = 0;
	kfxx::scope_guard release_vpgdesc_sg([pool, new_free_pg, &i]() noexcept {
		for (size_t j = 0; j < i; ++j) {
			kima_free_vpgdesc(pool, kima_lookup_vpgdesc(pool, ((char *)new_free_pg) + j * PAGESIZE));
		}
	});

	for (; i < PGROUNDUP(size); ++i) {
		kima_vpgdesc_t *vpgdesc = kima_alloc_vpgdesc(pool, ((char *)new_free_pg) + i * PAGESIZE);

		if (!vpgdesc)
			return NULL;
	}

	kima_ublk_t *ublk = kima_alloc_ublk(pool, new_free_pg, size);
	if (!ublk)
		return NULL;

	release_vpgdesc_sg.release();

	return new_free_pg;
}

PBOS_NODISCARD void *kima_realloc(kima_pool_t *pool, void *old_ptr, size_t size, size_t alignment) {
	kd_assert(size);
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
			j < PGCEIL(size);
			j += PAGESIZE) {
			// Look up for a continuous existing allocated virtual page area for allocation.
			if (!kima_lookup_vpgdesc(pool, ((char *)cur_desc->rb_value) + j)) {
				continuous_area_base = ((char *)cur_desc->rb_value) + j;
				it = kfxx::rbtree_t<void *>::iterator(pool->vpgdesc_query_tree.find_max_lteq(continuous_area_base), &pool->vpgdesc_query_tree, kfxx::iterator_direction::Forward);
				goto noncontinuous;
			}
		}

		{
			void *const limit = ((char *)cur_desc->rb_value) + (PGCEIL(size) - size);

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
					j < PGCEIL(size);
					j += PAGESIZE) {
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
				for (uintptr_t i = PGFLOOR(old_ptr);
					i < PGCEIL(((char *)old_ptr) + old_ublk->size);
					i += PAGESIZE) {
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
	char *new_free_pg = (char *)kima_vpgalloc(NULL, PGCEIL(size + alignment));

	if (size_t aligned_diff = ((uintptr_t)new_free_pg) % alignment; aligned_diff) {
		new_free_pg += alignment - aligned_diff;
	}

	if (!new_free_pg)
		return NULL;

	size_t i = 0;
	kfxx::scope_guard release_vpgdesc_sg([pool, new_free_pg, &i]() noexcept {
		for (size_t j = 0; j < i; ++j) {
			kima_free_vpgdesc(pool, kima_lookup_vpgdesc(pool, ((char *)new_free_pg) + j * PAGESIZE));
		}
	});

	for (; i < PGROUNDUP(size); ++i) {
		kima_vpgdesc_t *vpgdesc = kima_alloc_vpgdesc(pool, ((char *)new_free_pg) + i * PAGESIZE);

		if (!vpgdesc)
			return NULL;
	}

	kima_ublk_t *ublk = kima_alloc_ublk(pool, new_free_pg, size);
	if (!ublk)
		return NULL;

	release_vpgdesc_sg.release();

	return new_free_pg;
}
KI_EXPORT_IMAGE_SYMBOL(mm_krealloc);

PBOS_NODISCARD void *kima_realloc_in_place(kima_pool_t *pool, void *old_ptr, size_t size, size_t alignment) {
	kd_assert(size);
	char *continuous_area_base = nullptr;

	kima_ublk_t *old_ublk = static_cast<kima_ublk_t *>(pool->ublk_query_tree.find(old_ptr));
	kd_assert(old_ublk);

	kima_vpgdesc_t *cur_desc = static_cast<kima_vpgdesc_t *>(pool->vpgdesc_query_tree.find(old_ptr));

	for (size_t j = 0;
		j < PGCEIL(size);
		j += PAGESIZE) {
		// Look up for a continuous existing allocated virtual page area for allocation.
		if (!kima_lookup_vpgdesc(pool, ((char *)cur_desc->rb_value) + j)) {
			continuous_area_base = ((char *)cur_desc->rb_value) + j;
			return nullptr;
		}
	}

	{
		void *const limit = ((char *)cur_desc->rb_value) + (PGCEIL(size) - size);

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
			j < PGCEIL(size);
			j += PAGESIZE) {
			kima_vpgdesc_t *vpgdesc = kima_lookup_vpgdesc(pool, ((char *)cur_desc->rb_value) + j);

			if (!vpgdesc)
				km_panic("BUG: vpgdesc == nullptr, please report this bug to the developer");

			++vpgdesc->ref_count;
		}

		// Release unused old pages if the new size is smaller than the old size.
		for (size_t i = PGCEIL(old_ublk->size);
			i > PGCEIL(size);
			++i) {
			kima_vpgdesc_t *vpgdesc = kima_lookup_vpgdesc(pool, ((char*)old_ptr) + i);

			kd_assert(vpgdesc);

			if (!(--vpgdesc->ref_count))
				kima_free_vpgdesc(pool, vpgdesc);
		}

		old_ublk->size = size;

		pool->ublk_query_tree.insert_unwrap(old_ublk);

		return old_ptr;
	}

	// Allocate new pages if there is no suitable continuous virtual memory area.
	char *new_free_pg = (char *)kima_vpgalloc(NULL, PGCEIL(size + alignment));

	if (size_t aligned_diff = ((uintptr_t)new_free_pg) % alignment; aligned_diff) {
		new_free_pg += alignment - aligned_diff;
	}

	if (!new_free_pg)
		return NULL;

	size_t i = 0;
	kfxx::scope_guard release_vpgdesc_sg([pool, new_free_pg, &i]() noexcept {
		for (size_t j = 0; j < i; ++j) {
			kima_free_vpgdesc(pool, kima_lookup_vpgdesc(pool, ((char *)new_free_pg) + j * PAGESIZE));
		}
	});

	for (; i < PGROUNDUP(size); ++i) {
		kima_vpgdesc_t *vpgdesc = kima_alloc_vpgdesc(pool, ((char *)new_free_pg) + i * PAGESIZE);

		if (!vpgdesc)
			return NULL;
	}

	kima_ublk_t *ublk = kima_alloc_ublk(pool, new_free_pg, size);
	if (!ublk)
		return NULL;

	release_vpgdesc_sg.release();

	return new_free_pg;
}
KI_EXPORT_IMAGE_SYMBOL(mm_krealloc_in_place);

void kima_free(kima_pool_t *pool, void *ptr) {
	// io::irq_disable_lock irq_lock;

	kima_ublk_t *ublk = kima_lookup_ublk(pool, ptr);
	kd_assert(ublk);
	for (uintptr_t i = PGFLOOR(ublk->rb_value);
		i < PGCEIL(((char *)ublk->rb_value) + ublk->size);
		i += PAGESIZE) {
		kima_vpgdesc_t *vpgdesc = kima_lookup_vpgdesc(pool, (void *)i);

		kd_assert(vpgdesc);

		if (!(--vpgdesc->ref_count))
			kima_free_vpgdesc(pool, vpgdesc);
	}

	kima_free_ublk(pool, ublk);
}
KI_EXPORT_IMAGE_SYMBOL(mm_kfree);
