#include "pgalloc.hh"
#include <hal/i386/mm.h>
#include <pbos/km/logger.h>

PBOS_EXTERN_C_BEGIN

hn_madpool_t *hn_global_mad_pool_list = NULL;

void *mm_pgalloc(uint8_t memtype) {
	uint8_t pmemtype;
	if ((pmemtype = hn_to_kn_pmem_type(memtype)) == KN_PMEM_END)
		return NULL;

	pgaddr_t addr = hn_alloc_freeblk(pmemtype);
	if (ISVALIDPG(addr)) {
		hn_set_pgblk_used(addr, MAD_ALLOC_KERNEL);
		return UNPGADDR(addr);
	}

	return NULL;
}

void mm_pgfree(void *ptr) {
	hn_set_pgblk_free(PGROUNDDOWN(ptr));
}

void mm_refpg(void *ptr) {
	hn_set_pgblk_used(PGROUNDDOWN(ptr), MAD_ALLOC_KERNEL);
}

hn_mad_t *hn_get_mad(pgaddr_t pgaddr) {
	hn_pmad_t *pmad = hn_pmad_get(pgaddr);
	if (!pmad)
		km_panic("No PMAD corresponds to physical address %p", UNPGADDR(pgaddr));

	kfxx::rbtree_t<pgaddr_t>::node_t *mad;
	if ((mad = pmad->query_tree.find(pgaddr))) {
		return static_cast<hn_mad_t *>(mad);
	}

	km_panic("Physical memory block not found: %p", UNPGADDR(pgaddr));
}

///
/// @brief Mark a page as allocated.
///
/// @param pgaddr Paged address to mark.
/// @param type Allocation type.
///
void hn_set_pgblk_used(pgaddr_t pgaddr, uint8_t type) {
	hn_pmad_t *area = hn_pmad_get(pgaddr);
	hn_mad_t *mad = hn_get_mad(pgaddr);

	mad->flags = MAD_P;
	mad->type = type;

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

void hn_set_pgblk_free(pgaddr_t addr) {
	hn_pmad_t *area = hn_pmad_get(addr);
	hn_mad_t *mad = hn_get_mad(addr);

	if(!mad->ref_count)
		return;
	if (!(--mad->ref_count)) {
		mad->flags = MAD_P;
		mad->type = MAD_ALLOC_FREE;
		if(area->free_list)
			area->free_list->prev_free = mad;
		mad->next_free = area->free_list;
		kd_assert(!mad->prev_free);
	}
	return;
}

PBOS_EXTERN_C_END
