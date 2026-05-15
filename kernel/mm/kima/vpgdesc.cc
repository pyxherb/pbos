#include <string.h>
#include <pbos/ki/mm/kima.hh>

kima_vpgdesc_t *kima_lookup_vpgdesc(kima_pool_t *pool, void *ptr) {
	kfxx::RBTree<void *>::Node *node = pool->vpgdesc_query_tree.find(ptr);

	if (!node)
		return nullptr;

	return static_cast<kima_vpgdesc_t *>(node);
}

void kima_free_vpgdesc(kima_pool_t *pool, kima_vpgdesc_t *vpgdesc) {
	kima_vpgdesc_poolpg_t *poolpg = (kima_vpgdesc_poolpg_t *)kfxx::floor_align_to((uintptr_t)vpgdesc, pool->page_size);

	pool->vpgdesc_query_tree.remove(vpgdesc);
	vpgdesc->rb_value = vpgdesc;
	pool->vpgdesc_free_tree.insert_unwrap(vpgdesc);

	if (!(--poolpg->header.used_num)) {
		if (poolpg->header.prev)
			poolpg->header.prev->header.next = poolpg->header.next;
		if (poolpg->header.next)
			poolpg->header.next->header.prev = poolpg->header.prev;
		kima_vpgdesc_t *slots = (kima_vpgdesc_t *)(((uintptr_t)poolpg) + pool->vpgdesc_slots_off);
		for (size_t i = 0; i < pool->vpgdesc_slots_size; ++i) {
			pool->vpgdesc_free_tree.remove(&slots[i]);
		}
		kima_vpgfree(pool, poolpg, pool->page_size);
	}
}

kima_vpgdesc_t *kima_alloc_vpgdesc(kima_pool_t *pool, void *ptr) {
	if (pool->vpgdesc_free_tree.size()) {
		kima_vpgdesc_t *desc = static_cast<kima_vpgdesc_t *>(pool->vpgdesc_free_tree.begin().node);

		kima_ublk_poolpg_t *poolpg = (kima_ublk_poolpg_t *)kfxx::floor_align_to((uintptr_t)desc, pool->page_size);

		++poolpg->header.used_num;

		pool->vpgdesc_free_tree.remove(desc);

		desc->rb_value = ptr;
		desc->ref_count = 0;

		pool->vpgdesc_query_tree.insert_unwrap(desc);

		return desc;
	}

	kima_vpgdesc_poolpg_t *pg = (kima_vpgdesc_poolpg_t *)kima_vpgalloc(pool, NULL, pool->page_size);

	if (!pg)
		return nullptr;

	pg->header.prev = nullptr;
	pg->header.next = pool->vpgdesc_poolpg_list;
	if (pool->vpgdesc_poolpg_list) {
		pool->vpgdesc_poolpg_list->header.prev = pg;
	}
	pool->vpgdesc_poolpg_list = pg;

	pg->header.used_num = 1;

	kima_vpgdesc_t *slots = (kima_vpgdesc_t *)(((uintptr_t)pg) + pool->vpgdesc_slots_off);

	kfxx::construct_at<kima_vpgdesc_t>(&slots[0]);
	slots[0].rb_value = ptr;
	slots[0].ref_count = 0;

	pool->vpgdesc_query_tree.insert_unwrap(&slots[0]);

	for (size_t i = 1; i < pool->vpgdesc_slots_size; ++i) {
		kfxx::construct_at<kima_vpgdesc_t>(&slots[i]);

		slots[i].rb_value = &slots[i];

		pool->vpgdesc_free_tree.insert_unwrap(&slots[i]);
	}
	return &slots[0];
}
