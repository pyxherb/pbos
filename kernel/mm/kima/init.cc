#include <pbos/kd/logger.h>
#include <pbos/ki/mm/kima.hh>

void ki_init_kima_pool(kima_pool_t *pool) {
	const size_t page_size = mm_get_page_size();

	pool->vpgdesc_slots_off = kfxx::ceil_align_to((uintptr_t)sizeof(kima_vpgdesc_poolpg_header_t), alignof(kima_vpgdesc_t));
	pool->vpgdesc_slots_size = (page_size - pool->vpgdesc_slots_off) / sizeof(kima_vpgdesc_t);

	pool->ublk_slots_off = kfxx::ceil_align_to((uintptr_t)sizeof(kima_ublk_poolpg_header_t), alignof(kima_ublk_t));
	pool->ublk_slots_size = (page_size - pool->ublk_slots_off) / sizeof(kima_ublk_t);

	pool->page_size = page_size;
	pool->_initialized = true;
}

_kima_pool_t::_kima_pool_t() {
}

_kima_pool_t::~_kima_pool_t() {
	kima_free_pool(this);
}

void kima_free_pool(kima_pool_t *pool) {
	while (pool->ublk_query_tree.size())
		kima_free(pool, pool->ublk_query_tree.begin().node->rb_value);
}

size_t kima_get_allocated_page_num(kima_pool_t *pool) {
	return pool->num_allocated_pages;
}
