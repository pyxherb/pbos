#include <pbos/ki/mm/kima.hh>
#include <string.h>

kima_ublk_t* kima_lookup_ublk(kima_pool_t *pool, void* ptr) {
	kfxx::rbtree_t<void*>::node_t* node = pool->ublk_query_tree.find(ptr);

	if (!node)
		return nullptr;

	return static_cast<kima_ublk_t*>(node);
}

kima_ublk_t* kima_lookup_nearest_ublk(kima_pool_t *pool, void* ptr) {
	kfxx::rbtree_t<void*>::node_t* node = pool->ublk_query_tree.find_max_lteq(ptr);

	if (!node)
		return NULL;

	return static_cast<kima_ublk_t*>(node);
}

void kima_free_ublk(kima_pool_t *pool, kima_ublk_t *ublk) {
	kima_ublk_poolpg_t *poolpg = (kima_ublk_poolpg_t *)PGFLOOR(ublk);

	pool->ublk_query_tree.remove(ublk);

	if (!(--poolpg->header.used_num)) {
		if (poolpg->header.prev)
			poolpg->header.prev->header.next = poolpg->header.next;
		if (poolpg->header.next)
			poolpg->header.next->header.prev = poolpg->header.prev;
		kima_vpgfree(poolpg, PAGESIZE);
	}
}

kima_ublk_t *kima_alloc_ublk(kima_pool_t *pool, void *ptr, size_t size) {
	if (pool->ublk_free_tree.size()) {
		kima_ublk_t *desc = static_cast<kima_ublk_t *>(pool->ublk_free_tree.begin().node);

		pool->ublk_free_tree.remove(desc);

		desc->rb_value = ptr;
		desc->size = size;

		pool->ublk_query_tree.insert_unwrap(desc);

		return desc;
	}

	kima_ublk_poolpg_t *pg = (kima_ublk_poolpg_t *)kima_vpgalloc(NULL, PAGESIZE);

	if (!pg)
		return NULL;

	pg->header.prev = nullptr;
	pg->header.next = pool->ublk_poolpg_list;
	if (pool->ublk_poolpg_list) {
		pool->ublk_poolpg_list->header.prev = pg;
	}
	pool->ublk_poolpg_list = pg;

	pg->header.used_num = 1;

	kfxx::construct_at<kima_ublk_t>(&pg->slots[0]);

	pg->slots[0].rb_value = ptr;
	pg->slots[0].size = size;

	pool->ublk_query_tree.insert_unwrap(&pg->slots[0]);

	for (size_t i = 1; i < PBOS_ARRAYSIZE(pg->slots); ++i) {
		kfxx::construct_at<kima_ublk_t>(&pg->slots[i]);

		pg->slots[i].rb_value = &pg->slots[i];

		pool->ublk_free_tree.insert_unwrap(&pg->slots[i]);
	}
	return &pg->slots[0];
}
