#include "malloc.hh"

void *mm_kmalloc(size_t size) {
	kd_assert(size);
	void *filter_base = NULL;

	for (auto it = kima_vpgdesc_query_tree.begin(); it != kima_vpgdesc_query_tree.end(); ++it) {
		kima_vpgdesc_t *cur_desc = PB_CONTAINER_OF(kima_vpgdesc_t, node_header, it.node);

		if (cur_desc->node_header.value < filter_base)
			continue;

		for (size_t j = 0;
			 j < PGCEIL(size);
			 j += PAGESIZE) {
			if (!kima_lookup_vpgdesc(((char *)cur_desc->node_header.value) + j)) {
				filter_base = ((char *)cur_desc->node_header.value) + j;
				goto noncontinuous;
			}
		}

		{
			void *const limit = ((char *)cur_desc->node_header.value) + (PGCEIL(size) - size);

			for (void *cur_base = cur_desc->node_header.value;
				 cur_base <= limit;) {
				kima_ublk_t *nearest_ublk;
				if ((nearest_ublk = kima_lookup_nearest_ublk(cur_base))) {
					if (PB_ISOVERLAPPED((char *)cur_base, size, (char *)nearest_ublk->node_header.value, nearest_ublk->size)) {
						cur_base = ((char *)nearest_ublk->node_header.value) + nearest_ublk->size;
						continue;
					}
				}
				if ((nearest_ublk = kima_lookup_nearest_ublk(((char *)cur_base) + size - 1))) {
					if (PB_ISOVERLAPPED((char *)cur_base, size, (char *)nearest_ublk->node_header.value, nearest_ublk->size)) {
						cur_base = ((char *)nearest_ublk->node_header.value) + nearest_ublk->size;
						continue;
					}
				}

				kima_ublk_t *ublk = kima_alloc_ublk(cur_base, size);
				kd_assert(ublk);

				for (size_t j = 0;
					 j < PGCEIL(size);
					 j += PAGESIZE) {
					kima_vpgdesc_t *vpgdesc = kima_lookup_vpgdesc(((char *)cur_desc->node_header.value) + j);

					kd_assert(vpgdesc);

					++vpgdesc->ref_count;
				}

				return cur_base;
			}
		}

	noncontinuous:;
	}

	void *new_free_pg = kima_vpgalloc(NULL, PGCEIL(size));

	kd_assert(new_free_pg);

	for (size_t i = 0; i < PGROUNDUP(size); ++i) {
		kima_vpgdesc_t *vpgdesc = kima_alloc_vpgdesc(((char *)new_free_pg) + i * PAGESIZE);

		kd_assert(vpgdesc);
	}

	kima_ublk_t *ublk = kima_alloc_ublk(new_free_pg, size);
	kd_assert(ublk);

	return new_free_pg;
}

void mm_kfree(void *ptr) {
	kima_ublk_t *ublk = kima_lookup_ublk(ptr);
	kd_assert(ublk);
	for (uintptr_t i = PGFLOOR(ublk->node_header.value);
		 i < PGCEIL(((char *)ublk->node_header.value) + ublk->size);
		 i += PAGESIZE) {
		kima_vpgdesc_t *vpgdesc = kima_lookup_vpgdesc((void *)i);

		kd_assert(vpgdesc);

		if (!(--vpgdesc->ref_count))
			kima_free_vpgdesc(vpgdesc);
	}

	kima_free_ublk(ublk);
}
