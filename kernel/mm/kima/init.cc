#include <pbos/km/logger.h>
#include <pbos/ki/mm/kima.hh>

/*void ki_init_kima_pool(kima_pool_t *pool) {
	kd_printf("Initialized KIMA on address %p\n", pool);
}*/

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
