#include "vpgdesc.hh"

kima_vpgdesc_poolpg_t* kima_vpgdesc_poolpg_list = NULL;
kfxx::RBTree<void*> kima_vpgdesc_query_tree, kima_vpgdesc_free_tree;

void kima_vpgdesc_free(kima_vpgdesc_t* p) {
	kima_vpgdesc_t* _p = PB_CONTAINER_OF(kima_vpgdesc_t, node_header, p);

	_p->node_header.value = NULL;
}

kima_vpgdesc_t* kima_lookup_vpgdesc(void* ptr) {
	kfxx::RBTree<void*>::Node* node = kima_vpgdesc_query_tree.get(ptr);

	if (!node)
		return NULL;

	return PB_CONTAINER_OF(kima_vpgdesc_t, node_header, node);
}

void kima_free_vpgdesc(kima_vpgdesc_t* vpgdesc) {
	kima_vpgdesc_poolpg_t* poolpg = (kima_vpgdesc_poolpg_t*)PGFLOOR(vpgdesc);

	kima_vpgdesc_query_tree.remove(&vpgdesc->node_header);

	if (!(--poolpg->header.used_num)) {
		if (poolpg->header.prev)
			poolpg->header.prev->header.next = poolpg->header.next;
		if (poolpg->header.next)
			poolpg->header.next->header.prev = poolpg->header.prev;
		kima_vpgfree(poolpg, PAGESIZE);
	}
}

kima_vpgdesc_t* kima_alloc_vpgdesc(void* ptr) {
	if (kima_vpgdesc_free_tree.size()) {
		kima_vpgdesc_t* desc = PB_CONTAINER_OF(kima_vpgdesc_t, node_header, kima_vpgdesc_free_tree.begin().node);

		kima_vpgdesc_free_tree.remove(&desc->node_header);

		desc->node_header.value = ptr;
		desc->ref_count = 0;

		kima_vpgdesc_query_tree.insert(&desc->node_header);

		return desc;
	}

	kima_vpgdesc_poolpg_t* pg = (kima_vpgdesc_poolpg_t*)kima_vpgalloc(NULL, PAGESIZE);

	pg->header.prev = NULL;
	pg->header.next = kima_vpgdesc_poolpg_list;
	if (kima_vpgdesc_poolpg_list) {
		kima_vpgdesc_poolpg_list->header.prev = pg;
	}
	kima_vpgdesc_poolpg_list = pg;

	pg->header.used_num = 1;

	memset(&pg->slots[0].node_header, 0, sizeof(pg->slots[0].node_header));
	pg->slots[0].node_header.value = ptr;
	pg->slots[0].ref_count = 0;

	kima_vpgdesc_query_tree.insert(&pg->slots[0].node_header);

	for (size_t i = 1; i < PB_ARRAYSIZE(pg->slots); ++i) {
		memset(&pg->slots[i].node_header, 0, sizeof(pg->slots[i].node_header));
		pg->slots[i].node_header.value = &pg->slots[i];

		kima_vpgdesc_free_tree.insert(&pg->slots[i].node_header);
	}
	return &pg->slots[0];
}
