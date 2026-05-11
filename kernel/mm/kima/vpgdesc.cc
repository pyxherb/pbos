#include <string.h>
#include <pbos/ki/mm/kima.hh>

kima_vpgdesc_t *kima_lookup_vpgdesc(kima_pool_t *pool, void *ptr) {
	kfxx::rbtree_t<void *>::node_t *node = pool->vpgdesc_query_tree.find(ptr);

	if (!node)
		return nullptr;

	return static_cast<kima_vpgdesc_t *>(node);
}

void kima_free_vpgdesc(kima_pool_t *pool, kima_vpgdesc_t *vpgdesc) {
	kima_vpgdesc_poolpg_t *poolpg = (kima_vpgdesc_poolpg_t *)PGFLOOR(vpgdesc);

	pool->vpgdesc_query_tree.remove(vpgdesc);
	vpgdesc->rb_value = vpgdesc;
	pool->vpgdesc_free_tree.insert_unwrap(vpgdesc);

	if (!(--poolpg->header.used_num)) {
		if (poolpg->header.prev)
			poolpg->header.prev->header.next = poolpg->header.next;
		if (poolpg->header.next)
			poolpg->header.next->header.prev = poolpg->header.prev;
		for (auto &i : poolpg->slots) {
			pool->vpgdesc_free_tree.remove(&i);
		}
		kima_vpgfree(poolpg, PAGESIZE);
	}
}

kima_vpgdesc_t *kima_alloc_vpgdesc(kima_pool_t *pool, void *ptr) {
	if (pool->vpgdesc_free_tree.size()) {
		kima_vpgdesc_t *desc = static_cast<kima_vpgdesc_t *>(pool->vpgdesc_free_tree.begin().node);

		kima_ublk_poolpg_t *poolpg = (kima_ublk_poolpg_t *)PGFLOOR(desc);

		++poolpg->header.used_num;

		pool->vpgdesc_free_tree.remove(desc);

		desc->rb_value = ptr;
		desc->ref_count = 0;

		pool->vpgdesc_query_tree.insert_unwrap(desc);

		return desc;
	}

	kima_vpgdesc_poolpg_t *pg = (kima_vpgdesc_poolpg_t *)kima_vpgalloc(NULL, PAGESIZE);

	if (!pg)
		return nullptr;

	pg->header.prev = nullptr;
	pg->header.next = pool->vpgdesc_poolpg_list;
	if (pool->vpgdesc_poolpg_list) {
		pool->vpgdesc_poolpg_list->header.prev = pg;
	}
	pool->vpgdesc_poolpg_list = pg;

	pg->header.used_num = 1;

	kfxx::construct_at<kima_vpgdesc_t>(&pg->slots[0]);
	pg->slots[0].rb_value = ptr;
	pg->slots[0].ref_count = 0;

	pool->vpgdesc_query_tree.insert_unwrap(&pg->slots[0]);

	for (size_t i = 1; i < PBOS_ARRAYSIZE(pg->slots); ++i) {
		kfxx::construct_at<kima_vpgdesc_t>(&pg->slots[i]);

		pg->slots[i].rb_value = &pg->slots[i];

		pool->vpgdesc_free_tree.insert_unwrap(&pg->slots[i]);
	}
	return &pg->slots[0];
}
