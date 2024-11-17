#include <hal/i386/mm.h>
#include <pbos/km/logger.h>

void *mm_kmalloc(size_t size) {
	assert(size);
	void *filter_base = NULL;

	kf_rbtree_foreach(i, &kima_vpgdesc_query_tree) {
		kima_vpgdesc_t *cur_desc = PB_CONTAINER_OF(kima_vpgdesc_t, node_header, i);

		if (cur_desc->ptr < filter_base)
			continue;

		for (size_t j = 0;
			 j < PGCEIL(size);
			 j += PAGESIZE) {
			if (!kima_lookup_vpgdesc(((char *)cur_desc->ptr) + j)) {
				filter_base = ((char *)cur_desc->ptr) + j;
				goto noncontinuous;
			}
		}

		{
			void *const limit = ((char *)cur_desc->ptr) + (PGCEIL(size) - size);

			for (void *cur_base = cur_desc->ptr;
				 cur_base <= limit;) {
				kima_ublk_t *nearest_ublk;
				if ((nearest_ublk = kima_lookup_nearest_ublk(cur_base))) {
					if (PB_ISOVERLAPPED((char *)cur_base, size, (char *)nearest_ublk->ptr, nearest_ublk->size)) {
						cur_base = ((char *)nearest_ublk->ptr) + nearest_ublk->size;
						continue;
					}
				}
				if ((nearest_ublk = kima_lookup_nearest_ublk(((char *)cur_base) + size - 1))) {
					if (PB_ISOVERLAPPED((char *)cur_base, size, (char *)nearest_ublk->ptr, nearest_ublk->size)) {
						cur_base = ((char *)nearest_ublk->ptr) + nearest_ublk->size;
						continue;
					}
				}

				kima_ublk_t *ublk = kima_alloc_ublk(cur_base, size);
				assert(ublk);

				for (size_t j = 0;
					 j < PGCEIL(size);
					 j += PAGESIZE) {
					kima_vpgdesc_t *vpgdesc = kima_lookup_vpgdesc(((char *)cur_desc->ptr) + j);

					assert(vpgdesc);

					++vpgdesc->ref_count;
				}

				return cur_base;
			}
		}

	noncontinuous:;
	}

	void *new_free_pg = kima_vpgalloc(NULL, PGCEIL(size));

	assert(new_free_pg);

	for (size_t i = 0; i < PGROUNDUP(size); ++i) {
		kima_vpgdesc_t *vpgdesc = kima_alloc_vpgdesc(((char *)new_free_pg) + i * PAGESIZE);

		assert(vpgdesc);
	}

	kima_ublk_t *ublk = kima_alloc_ublk(new_free_pg, size);
	assert(ublk);

	return new_free_pg;
}

void mm_kfree(void *ptr) {
	kima_ublk_t *ublk = kima_lookup_ublk(ptr);
	assert(ublk);
	for (uintptr_t i = PGFLOOR(ublk->ptr);
		 i < PGCEIL(((char *)ublk->ptr) + ublk->size);
		 i += PAGESIZE) {
		kima_vpgdesc_t *vpgdesc = kima_lookup_vpgdesc((void *)i);

		assert(vpgdesc);

		if (!(--vpgdesc->ref_count))
			kima_free_vpgdesc(vpgdesc);
	}

	kima_free_ublk(ublk);
}
