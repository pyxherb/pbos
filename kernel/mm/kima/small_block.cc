#include <pbos/kd/logger.h>
#include <pbos/ki/mm/kima.hh>

PBOS_EXTERN_C_BEGIN

size_t kima_calc_small_block_index(kima_pool_t *pool, size_t order, size_t off) {
	return off / KIMA_ORDERED_BLOCK_SIZE(order);
}

size_t kima_calc_small_block_capacity(kima_pool_t *pool, size_t order) {
	static_assert(KIMA_ORDERED_BLOCK_SIZE(KIMA_SMALL_BLOCK_MIN_ORDER) >= alignof(kima_small_block_desc_t));
	// (c + d) * x = p
	// Where x is the total block number, c is the ordered block size in bits, p is the available page size, d is the descriptor size.
	return ((pool->page_size - sizeof(kima_small_block_page_desc_t) * 2) / (KIMA_ORDERED_BLOCK_SIZE(order) + (sizeof(kima_small_block_desc_t) * 2)));
}

void kima_calc_small_block_page_info(kima_pool_t *pool, size_t order, kima_small_block_page_info_t *info_out) {
	size_t capacity = kima_calc_small_block_capacity(pool, order);

	info_out->max_block_capacity = capacity;

	info_out->block_descs_off = kfxx::ceil_align_to(capacity * KIMA_ORDERED_BLOCK_SIZE(order), alignof(kima_small_block_desc_t));
	info_out->page_descs_off = kfxx::floor_align_to(pool->page_size - sizeof(kima_small_block_desc_t), alignof(kima_small_block_page_desc_t));
}

void *kima_alloc_small_block_page(kima_pool_t *pool, size_t order) {
	char *pg = (char *)kima_vpgalloc(pool, NULL, pool->page_size);

	if (!pg)
		return nullptr;

	auto &info = pool->small_block_page_info[order - KIMA_SMALL_BLOCK_MIN_ORDER];
	kima_small_block_page_desc_t *page_desc =
		(kima_small_block_page_desc_t *)(pg + info.page_descs_off);

	kfxx::construct_at<kima_small_block_page_desc_t>(page_desc);

	ps::mutex_guard g(pool->mutex.c_mutex());

	// Add the new block page to the pool.
	page_desc->next = pool->small_block_pages;
	page_desc->order = order;
	if (pool->small_block_pages)
		pool->small_block_pages->prev = page_desc;
	pool->small_block_pages = page_desc;

	// Link the descriptors.
	size_t index = order - KIMA_SMALL_BLOCK_MIN_ORDER;
	auto prev_free_small_block_descs_head = pool->free_small_block_descs[index];
	for (size_t i = 0; i < info.max_block_capacity; ++i) {
		kima_small_block_desc_t *const desc = &((kima_small_block_desc_t *)(pg + info.block_descs_off))[i];
		kfxx::construct_at<kima_small_block_desc_t>(desc);
		desc->is_free = true;
		if (i != 0) {
			desc->prev = desc - 1;
		} else {
			pool->free_small_block_descs[index] = desc;
			desc->prev = nullptr;
		}

		if (i + 1 == info.max_block_capacity) {
			desc->next = prev_free_small_block_descs_head;
			if (prev_free_small_block_descs_head)
				prev_free_small_block_descs_head->prev = desc;
		} else {
			desc->next = desc + 1;
		}

		desc->ptr = pg + KIMA_ORDERED_BLOCK_SIZE(order) * i;

		if (desc->ptr == (char *)0xffffc00000c29200)
			klog_printf("");
	}

	return pg;
}

void kima_free_small_block_desc(kima_pool_t *pool, kima_small_block_page_desc_t *page_desc, kima_small_block_desc_t *desc) {
	size_t index = page_desc->order - KIMA_SMALL_BLOCK_MIN_ORDER;

	kd_assert(!desc->is_free);

	desc->is_free = true;

	// Insert the descriptor back to the free list.
	if (desc->next)
		desc->next->prev = desc->prev;
	if (desc->prev)
		desc->prev->next = desc->next;
	kd_assert(!pool->used_small_block_descs[index]->prev);
	if (desc == pool->used_small_block_descs[index])
		pool->used_small_block_descs[index] = desc->next;

	kd_assert(!pool->free_small_block_descs[index]->prev);
	desc->prev = nullptr;
	if (pool->free_small_block_descs[index])
		pool->free_small_block_descs[index]->prev = desc;
	desc->next = pool->free_small_block_descs[index];
	pool->free_small_block_descs[index] = desc;

	// TODO: Free it and modify the list.
	if (!(--page_desc->num_used)) {
		if (page_desc->prev)
			page_desc->prev->next = page_desc->next;
		if (page_desc->next)
			page_desc->next->prev = page_desc->prev;
		if (page_desc == pool->small_block_pages)
			pool->small_block_pages = page_desc->next;

		const size_t capacity = pool->small_block_page_info[index].max_block_capacity;

		uintptr_t floor_addr = kfxx::floor_align_to((uintptr_t)desc, pool->page_size);
		kima_small_block_desc_t *descs = (kima_small_block_desc_t *)(floor_addr + pool->small_block_page_info[index].block_descs_off);

		for (size_t i = 0; i < capacity; ++i) {
			if (descs[i].next)
				descs[i].next->prev = descs[i].prev;
			if (descs[i].prev)
				descs[i].prev->next = descs[i].next;
			if (&descs[i] == pool->free_small_block_descs[index])
				pool->free_small_block_descs[index] = descs[i].next;
		}
		kima_vpgfree(pool, (void *)kfxx::floor_align_to((uintptr_t)page_desc, pool->page_size), pool->page_size);
	}
}

kima_small_block_desc_t *kima_alloc_small_block_desc(kima_pool_t *pool, size_t order) {
	size_t index = order - KIMA_SMALL_BLOCK_MIN_ORDER;
	if (!pool->free_small_block_descs[index]) {
		if (!kima_alloc_small_block_page(pool, order))
			return nullptr;
	}
	kd_assert(pool->free_small_block_descs[index]->is_free);

	kima_small_block_desc_t *desc = pool->free_small_block_descs[index];

	kd_assert(!desc->prev);
	kd_assert(desc->is_free);

	if (desc->next)
		desc->next->prev = nullptr;
	pool->free_small_block_descs[index] = desc->next;

	desc->prev = nullptr;
	desc->next = pool->used_small_block_descs[index];
	if (pool->used_small_block_descs[index])
		pool->used_small_block_descs[index]->prev = desc;
	pool->used_small_block_descs[index] = desc;

	desc->is_free = false;

	uintptr_t ceil_addr = kfxx::floor_align_to((uintptr_t)desc, pool->page_size) + pool->page_size;
	kima_small_block_page_desc_t *page_desc = (kima_small_block_page_desc_t *)(ceil_addr - sizeof(kima_small_block_desc_t));

	++page_desc->num_used;

	return desc;
}

PBOS_EXTERN_C_END
