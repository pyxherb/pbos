#include "vpgdesc.hh"
#include <string.h>

kima_vpgdesc_poolpg_t* kima_vpgdesc_poolpg_list = NULL;
kfxx::rbtree_t<void*> kima_vpgdesc_query_tree, kima_vpgdesc_free_tree;

void kima_free_vpgdesc(kima_vpgdesc_t* vpgdesc) {
	kima_vpgdesc_poolpg_t* poolpg = (kima_vpgdesc_poolpg_t*)PGFLOOR(vpgdesc);

	kima_vpgdesc_query_tree.remove(vpgdesc);

	if (!(--poolpg->header.used_num)) {
		if (poolpg->header.prev)
			poolpg->header.prev->header.next = poolpg->header.next;
		if (poolpg->header.next)
			poolpg->header.next->header.prev = poolpg->header.prev;
		kima_vpgfree(poolpg, DEFAULT_PAGESIZE);
	}
}

kima_vpgdesc_t* kima_alloc_vpgdesc(void* ptr) {
	if (kima_vpgdesc_free_tree.size()) {
		kima_vpgdesc_t* desc = static_cast<kima_vpgdesc_t*>(kima_vpgdesc_free_tree.begin().node);

		kima_vpgdesc_free_tree.remove(desc);

		desc->rb_value = ptr;
		desc->ref_count = 0;

		kima_vpgdesc_query_tree.insert(desc);

		return desc;
	}

	kima_vpgdesc_poolpg_t* pg = (kima_vpgdesc_poolpg_t*)kima_vpgalloc(NULL, DEFAULT_PAGESIZE);

	pg->header.prev = NULL;
	pg->header.next = kima_vpgdesc_poolpg_list;
	if (kima_vpgdesc_poolpg_list) {
		kima_vpgdesc_poolpg_list->header.prev = pg;
	}
	kima_vpgdesc_poolpg_list = pg;

	pg->header.used_num = 1;

	pg->slots[0].rb_value = ptr;
	pg->slots[0].ref_count = 0;

	kima_vpgdesc_query_tree.insert(&pg->slots[0]);

	for (size_t i = 1; i < PBOS_ARRAYSIZE(pg->slots); ++i) {
		pg->slots[i].rb_value = &pg->slots[i];

		kima_vpgdesc_free_tree.insert(&pg->slots[i]);
	}
	return &pg->slots[0];
}
