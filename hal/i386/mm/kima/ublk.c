#include "ublk.h"

kima_ublk_poolpg_t *kima_ublk_poolpg_list = NULL;
kf_rbtree_t kima_ublk_query_tree;

bool kima_ublk_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y) {
	kima_ublk_t *_x = PB_CONTAINER_OF(kima_ublk_t, node_header, x),
				*_y = PB_CONTAINER_OF(kima_ublk_t, node_header, y);

	return _x->ptr < _y->ptr;
}

void kima_ublk_nodefree(kf_rbtree_node_t *p) {
	kima_ublk_t *_p = PB_CONTAINER_OF(kima_ublk_t, node_header, p);

	_p->ptr = NULL;
}

kima_ublk_t *kima_lookup_ublk(void *ptr) {
	kima_ublk_t query_desc = {
		.ptr = ptr
	};

	kf_rbtree_node_t *node = kf_rbtree_find(&kima_ublk_query_tree, &query_desc);

	if (!node)
		return NULL;

	return PB_CONTAINER_OF(kima_ublk_t, node_header, node);
}

kima_ublk_t *kima_lookup_nearest_ublk(void *ptr) {
	kima_ublk_t query_desc = {
		.ptr = ptr
	};

	kf_rbtree_node_t *node = kf_rbtree_find_max_lteq_node(&kima_ublk_query_tree, &query_desc.node_header);

	if (!node)
		return NULL;

	return PB_CONTAINER_OF(kima_ublk_t, node_header, node);
}

void kima_free_ublk(kima_ublk_t *ublk) {
	kima_ublk_poolpg_t *poolpg = (kima_ublk_poolpg_t *)PGFLOOR(ublk);

	kf_rbtree_remove(&kima_ublk_query_tree, &ublk->node_header);

	if (!(--poolpg->header.used_num)) {
		if (poolpg->header.prev)
			poolpg->header.prev->header.next = poolpg->header.next;
		if (poolpg->header.next)
			poolpg->header.next->header.prev = poolpg->header.prev;
		kima_vpgfree(poolpg, PAGESIZE);
	}
}

kima_ublk_t *kima_alloc_ublk(void *ptr, size_t size) {
	for (kima_ublk_poolpg_t *pg = kima_ublk_poolpg_list;
		 pg;
		 pg = pg->header.next) {
		if (pg->header.used_num >= PB_ARRAYSIZE(pg->slots)) {
			continue;
		}
		for (size_t i = 0; i < PB_ARRAYSIZE(pg->slots); ++i) {
			if (!pg->slots[i].ptr) {
				++pg->header.used_num;
				pg->slots[i].ptr = ptr;
				pg->slots[i].size = size;
				memset(&pg->slots[i].node_header, 0, sizeof(pg->slots[i].node_header));
				kf_rbtree_insert(&kima_ublk_query_tree, &pg->slots[i].node_header);
				return &pg->slots[i];
			}
		}
	}

	kima_ublk_poolpg_t *pg = kima_vpgalloc(NULL, PAGESIZE);

	memset(pg->slots, 0, sizeof(pg->slots));

	pg->header.next = kima_ublk_poolpg_list;
	if (kima_ublk_poolpg_list) {
		kima_ublk_poolpg_list->header.prev = pg;
	}
	kima_ublk_poolpg_list = pg;

	pg->header.used_num = 1;
	pg->slots[0].ptr = ptr;
	pg->slots[0].size = size;
	memset(&pg->slots[0].node_header, 0, sizeof(pg->slots[0].node_header));
	kf_rbtree_insert(&kima_ublk_query_tree, &pg->slots[0].node_header);
	return &pg->slots[0];
}
