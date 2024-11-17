#include "pgalloc.h"
#include <hal/i386/mm.h>
#include <math.h>
#include <pbos/km/logger.h>

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

///
/// @brief Find a MAD with specified order which manages the address.
///
/// @param pool Target pool.
/// @param pgaddr Target paged address.
/// @param order Target order.
/// @return Found MAD. NULL if not found.
///
hn_mad_t *hn_find_mad(hn_madpool_t *pool, uint32_t pgaddr) {
	for (uint16_t i = 0; i < PB_ARRAYSIZE(pool->descs); ++i) {
		// There's no more MAD once we found that a MAD does not present.
		if (!(pool->descs[i].flags & MAD_P))
			break;
		if (ISINRANGE(pool->descs[i].pgaddr, 1, pgaddr)) {
			return &(pool->descs[i]);
		}
	}

	return NULL;
}

///
/// @brief Mark a page as allocated.
///
/// @param pgaddr Paged address to mark.
/// @param type Allocation type.
///
void hn_set_pgblk_used(pgaddr_t pgaddr, uint8_t type) {
	// kprintf("hn_set_pgblk_used: %p (%d)\n", UNPGADDR(pgaddr), (int)order & 0x3f);
	hn_pmad_t *pmad = hn_pmad_get(pgaddr);
	assert(pmad);

	hn_madpool_t *pool = pmad->madpools;
	while (pool) {
		hn_mad_t *mad = hn_find_mad(pool, pgaddr);
		if (mad) {
			pgaddr_t addr = mad->pgaddr;

			mad->flags = MAD_P;
			mad->type = type;
			++mad->ref_count;

			return;
		}

		assert(pool != pool->next);
		pool = pool->next;
	}

	assert(false);
}

void hn_set_pgblk_free(pgaddr_t addr) {
	hn_pmad_t *const pmad = hn_pmad_get(addr);
	assert(pmad);

	hn_madpool_t *pool = pmad->madpools;
	while (pool) {
		hn_mad_t *mad = hn_find_mad(pool, addr);
		if (mad) {
			assert(mad->ref_count);
			if (!(--mad->ref_count)) {
				mad->flags = MAD_P;
				mad->type = MAD_ALLOC_FREE;
			}
			return;
		}

		assert(pool != pool->next);
		pool = pool->next;
	}

	assert(false);
}
