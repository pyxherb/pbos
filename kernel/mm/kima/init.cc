#include <pbos/kd/logger.h>
#include <pbos/kf/misc.h>
#include <pbos/ki/mm/kima.hh>

void kima_init_pool(kima_pool_t *pool) {
	const size_t page_size = mm_get_page_size();

	pool->vpgdesc_slots_off = kfxx::ceil_align_to((uintptr_t)sizeof(kima_vpgdesc_poolpg_header_t), alignof(kima_vpgdesc_t));
	pool->vpgdesc_slots_size = (page_size - pool->vpgdesc_slots_off) / sizeof(kima_vpgdesc_t);

	pool->ublk_slots_off = kfxx::ceil_align_to((uintptr_t)sizeof(kima_ublk_poolpg_header_t), alignof(kima_ublk_t));
	pool->ublk_slots_size = (page_size - pool->ublk_slots_off) / sizeof(kima_ublk_t);

	pool->page_size = page_size;

	for (size_t i = KIMA_SMALL_BLOCK_MIN_ORDER, j = 0; i <= KIMA_SMALL_BLOCK_MAX_ORDER; ++i, ++j) {
		pool->small_block_page_info[j] = {};
		pool->free_small_block_descs[j] = nullptr;
		pool->used_small_block_descs[j] = nullptr;
		kima_calc_small_block_page_info(pool, i, &pool->small_block_page_info[j]);
	}
	pool->_initialized = true;
}

_kima_pool_t::_kima_pool_t() {
}

_kima_pool_t::_kima_pool_t(_kima_pool_t &&rhs) {
	if (rhs._initialized) {
		ps::mutex_guard g(rhs.mutex);

		ublk_poolpg_list = rhs.ublk_poolpg_list;
		rhs.ublk_poolpg_list = nullptr;

		ublk_query_tree = std::move(rhs.ublk_query_tree);
		ublk_free_tree = std::move(rhs.ublk_free_tree);

		vpgdesc_poolpg_list = rhs.vpgdesc_poolpg_list;
		rhs.vpgdesc_poolpg_list = nullptr;

		vpgdesc_query_tree = std::move(rhs.vpgdesc_query_tree);
		vpgdesc_free_tree = std::move(rhs.vpgdesc_free_tree);

		num_allocated_pages = rhs.num_allocated_pages;

		page_size = rhs.page_size;

		ublk_slots_off = rhs.ublk_slots_off;
		ublk_slots_size = rhs.ublk_slots_size;

		vpgdesc_slots_off = rhs.vpgdesc_slots_off;
		vpgdesc_slots_size = rhs.vpgdesc_slots_size;

		_initialized = true;
	} else
		_initialized = false;
}

_kima_pool_t::~_kima_pool_t() {
	if (_initialized)
		kima_free_pool(this);
}

void kima_free_pool(kima_pool_t *pool) {
	while (pool->ublk_query_tree.size())
		kima_free(pool, pool->ublk_query_tree.begin().node->rb_value);
}

size_t kima_get_allocated_page_num(kima_pool_t *pool) {
	return pool->num_allocated_pages;
}
