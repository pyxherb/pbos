#include "ublk.hh"

kima_ublk_poolpg_t* kima_ublk_poolpg_list = NULL;
kfxx::rbtree_t<void*> kima_ublk_query_tree, kima_ublk_free_tree;

kima_ublk_t* kima_lookup_ublk(void* ptr) {
	kfxx::rbtree_t<void*>::node_t* node = kima_ublk_query_tree.find(ptr);

	if (!node)
		return nullptr;

	return static_cast<kima_ublk_t*>(node);
}

kima_ublk_t* kima_lookup_nearest_ublk(void* ptr) {
	kfxx::rbtree_t<void*>::node_t* node = kima_ublk_query_tree.find_max_lteq(ptr);

	if (!node)
		return NULL;

	return static_cast<kima_ublk_t*>(node);
}

void kima_free_ublk(kima_ublk_t* ublk) {
	kima_ublk_poolpg_t* poolpg = (kima_ublk_poolpg_t*)PGFLOOR(ublk);

	kima_ublk_query_tree.remove(ublk);

	if (!(--poolpg->header.used_num)) {
		if (poolpg->header.prev)
			poolpg->header.prev->header.next = poolpg->header.next;
		if (poolpg->header.next)
			poolpg->header.next->header.prev = poolpg->header.prev;
		kima_vpgfree(poolpg, PAGESIZE);
	}
}

kima_ublk_t* kima_alloc_ublk(void* ptr, size_t size) {
	if (kima_ublk_free_tree.size()) {
		kima_ublk_t* desc = static_cast<kima_ublk_t*>(kima_ublk_free_tree.begin().node);

		kima_ublk_free_tree.remove(desc);

		desc->rb_value = ptr;
		desc->size = size;

		kima_ublk_query_tree.insert(desc);

		return desc;
	}

	kima_ublk_poolpg_t* pg = (kima_ublk_poolpg_t*)kima_vpgalloc(NULL, PAGESIZE);

	pg->header.next = kima_ublk_poolpg_list;
	if (kima_ublk_poolpg_list) {
		kima_ublk_poolpg_list->header.prev = pg;
	}
	kima_ublk_poolpg_list = pg;

	pg->header.used_num = 1;

	memset(&pg->slots[0], 0, sizeof(pg->slots[0]));
	pg->slots[0].rb_value = ptr;
	pg->slots[0].size = size;

	kima_ublk_query_tree.insert(&pg->slots[0]);

	for (size_t i = 1; i < PBOS_ARRAYSIZE(pg->slots); ++i) {
		memset(&pg->slots[i], 0, sizeof(pg->slots[i]));
		pg->slots[i].rb_value = &pg->slots[i];

		kima_ublk_free_tree.insert(&pg->slots[i]);
	}
	return &pg->slots[0];
}
