#include <pbos/kfxx/scope_guard.hh>
#include <pbos/ki/mm/pgalloc.hh>

PBOS_EXTERN_C_BEGIN

ki_pmad_t ki_initial_pmad_storage[KI_INITIAL_MM_AREA_STORAGE_NUM];
kfxx::rbtree_t<void *> ki_pmad_tree;
size_t ki_pmad_number;
hali_madpool_t *ki_global_mad_pool_list = NULL;

size_t kh_mad_pool_descs_off, kh_mad_pool_descs_num_per_page;

ki_pmad_t::ki_pmad_t() {
}

void *mm_pgalloc(uint8_t memtype) {
	void *addr = ki_alloc_free_page(memtype);
	if (addr) {
		ki_set_page_used(addr);
		return addr;
	}

	return NULL;
}

void mm_pgfree(void *ptr) {
	ki_set_page_free(ptr);
}

void mm_refpg(void *ptr) {
	ki_set_page_used(ptr);
}

hali_mad_t *kh_get_mad(void *pgaddr) {
	ki_pmad_t *pmad = ki_get_pmad(pgaddr);
	if (!pmad)
		km_panic("No PMAD corresponds to physical address %p", pgaddr);

	kfxx::rbtree_t<void *>::node_t *mad;
	if ((mad = pmad->query_tree.find(pgaddr))) {
		return static_cast<hali_mad_t *>(mad);
	}

	km_panic("Physical memory block not found: %p", pgaddr);
}

///
/// @brief Mark a page as allocated.
///
/// @param pgaddr Paged address to mark.
/// @param type Allocation type.
///
void ki_set_page_used(void *pgaddr) {
	ki_pmad_t *area = ki_get_pmad(pgaddr);
	hali_mad_t *mad = kh_get_mad(pgaddr);

	hal_lock_spinlock(&mad->spinlock);
	kfxx::deferred unlocker([mad]() noexcept {
		hal_unlock_spinlock(&mad->spinlock);
	});

	if (!(mad->ref_count++)) {
		if (mad == area->free_list)
			area->free_list = mad->next_free;
		if (mad->prev_free)
			mad->prev_free->next_free = mad->next_free;
		if (mad->next_free)
			mad->next_free->prev_free = mad->prev_free;
		mad->prev_free = nullptr;
		mad->next_free = nullptr;
	}
}

void ki_set_page_free(void *addr) {
	ki_pmad_t *area = ki_get_pmad(addr);
	hali_mad_t *mad = kh_get_mad(addr);

	hal_lock_spinlock(&mad->spinlock);
	kfxx::deferred unlocker([mad]() noexcept {
		hal_unlock_spinlock(&mad->spinlock);
	});

	if (!mad->ref_count)
		km_panic("Freeing physical page at %p with reference count == 0", addr);
	if (!(--mad->ref_count)) {
		if (area->free_list)
			area->free_list->prev_free = mad;
		mad->next_free = area->free_list;
		kd_assert(!mad->prev_free);
	}
	return;
}

///
/// @brief Get PMAD by a physical address.
///
/// @param addr Physical address to find.
/// @return Corresponding PMAD. NULL if not found.
///
ki_pmad_t *ki_get_pmad(void *addr) {
	auto node = ki_pmad_tree.find_max_lteq(addr);

	if (node) {
		if (((char *)node->rb_value) + (static_cast<ki_pmad_t *>(node)->len) > addr)
			return static_cast<ki_pmad_t *>(node);
	}

	return nullptr;
}

void *ki_alloc_free_page(uint8_t type) {
	KI_PMAD_FOREACH(i) {
		if (i->type != type)
			continue;
		if (i->free_list)
			return i->free_list->rb_value;
	}

	return nullptr;
}


PBOS_EXTERN_C_END
