#include "ublk.hh"

kima_ublk_poolpg_t* kima_ublk_poolpg_list = NULL;
kfxx::RBTree<void*> kima_ublk_query_tree, kima_ublk_free_tree;

void kima_ublk_free(kima_ublk_t* p) {
	p->node_header.value = NULL;
}

kima_ublk_t* kima_lookup_ublk(void* ptr) {
	kfxx::RBTree<void*>::Node* node = kima_ublk_query_tree.get(ptr);

	if (!node)
		return nullptr;

	return PB_CONTAINER_OF(kima_ublk_t, node_header, node);
}

kima_ublk_t* kima_lookup_nearest_ublk(void* ptr) {
	kfxx::RBTree<void*>::Node* node = kima_ublk_query_tree.getMaxLteqNode(ptr);

	if (!node)
		return NULL;

	return PB_CONTAINER_OF(kima_ublk_t, node_header, node);
}

void kima_free_ublk(kima_ublk_t* ublk) {
	kima_ublk_poolpg_t* poolpg = (kima_ublk_poolpg_t*)PGFLOOR(ublk);

	kima_ublk_query_tree.remove(&ublk->node_header);

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
		kima_ublk_t* desc = PB_CONTAINER_OF(kima_ublk_t, node_header, kima_ublk_free_tree.begin().node);

		kima_ublk_free_tree.remove(&desc->node_header);

		desc->node_header.value = ptr;
		desc->size = size;

		kima_ublk_query_tree.insert(&desc->node_header);

		return desc;
	}

	kima_ublk_poolpg_t* pg = (kima_ublk_poolpg_t*)kima_vpgalloc(NULL, PAGESIZE);

	pg->header.next = kima_ublk_poolpg_list;
	if (kima_ublk_poolpg_list) {
		kima_ublk_poolpg_list->header.prev = pg;
	}
	kima_ublk_poolpg_list = pg;

	pg->header.used_num = 1;

	memset(&pg->slots[0].node_header, 0, sizeof(pg->slots[0].node_header));
	pg->slots[0].node_header.value = ptr;
	pg->slots[0].size = size;
	
	kima_ublk_query_tree.insert(&pg->slots[0].node_header);

	for (size_t i = 1; i < PB_ARRAYSIZE(pg->slots); ++i) {
		memset(&pg->slots[i].node_header, 0, sizeof(pg->slots[i].node_header));
		pg->slots[i].node_header.value = &pg->slots[i];

		kima_ublk_free_tree.insert(&pg->slots[i].node_header);
	}
	return &pg->slots[0];
}
