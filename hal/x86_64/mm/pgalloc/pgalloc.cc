#include "pgalloc.hh"
#include <pbos/km/logger.h>
#include <hal/x86_64/mm.hh>
#include <pbos/kfxx/scope_guard.hh>

PBOS_EXTERN_C_BEGIN

hn_madpool_t *hn_global_mad_pool_list = NULL;

void *mm_pgalloc(uint8_t memtype) {
	void *addr = hn_alloc_freeblk(memtype);
	if (addr) {
		hn_set_pgblk_used(addr);
		return addr;
	}

	return NULL;
}

void mm_pgfree(void *ptr) {
	hn_set_pgblk_free(ptr);
}

void mm_refpg(void *ptr) {
	hn_set_pgblk_used(ptr);
}

hn_mad_t *hn_get_mad(void *pgaddr) {
	hn_pmad_t *pmad = hn_pmad_get(pgaddr);
	if (!pmad)
		km_panic("No PMAD corresponds to physical address %p", pgaddr);

	kfxx::rbtree_t<void *>::node_t *mad;
	if ((mad = pmad->query_tree.find(pgaddr))) {
		return static_cast<hn_mad_t *>(mad);
	}

	km_panic("Physical memory block not found: %p", pgaddr);
}

///
/// @brief Mark a page as allocated.
///
/// @param pgaddr Paged address to mark.
/// @param type Allocation type.
///
void hn_set_pgblk_used(void *pgaddr) {
	hn_pmad_t *area = hn_pmad_get(pgaddr);
	hn_mad_t *mad = hn_get_mad(pgaddr);

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

void hn_set_pgblk_free(void *addr) {
	hn_pmad_t *area = hn_pmad_get(addr);
	hn_mad_t *mad = hn_get_mad(addr);

	hal_lock_spinlock(&mad->spinlock);
	kfxx::deferred unlocker([mad]() noexcept {
		hal_unlock_spinlock(&mad->spinlock);
	});

	if (!mad->ref_count)
		return;
	if (!(--mad->ref_count)) {
		if (area->free_list)
			area->free_list->prev_free = mad;
		mad->next_free = area->free_list;
		kd_assert(!mad->prev_free);
	}
	return;
}

PBOS_EXTERN_C_END
