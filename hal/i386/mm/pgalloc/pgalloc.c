#include "pgalloc.h"
#include <hal/i386/mm.h>
#include <math.h>
#include <pbos/km/logger.h>

static hn_mad_t *_hn_find_mad(hn_madpool_t *pool, uint32_t pgaddr, uint8_t order);

#define LHINT 0x80	// Lesser Hint
#define GHINT 0x40	// Greater Hint

void *mm_pgalloc(uint8_t memtype, uint8_t order) {
	assert(order <= MM_MAXORD);

	uint8_t pmemtype;
	if ((pmemtype = hn_to_kn_pmem_type(memtype)) == KN_PMEM_END)
		return NULL;

	pgaddr_t addr = hn_alloc_freeblk(pmemtype, order);
	if (ISVALIDPG(addr)) {
		hn_set_pgblk_used(addr, MAD_ALLOC_KERNEL, order);
		return UNPGADDR(addr);
	}

	return NULL;
}

void mm_pgfree(void *ptr, uint8_t order) {
	assert(order <= MM_MAXORD);
	hn_set_pgblk_free(PGROUNDDOWN(ptr), order);
}

void mm_refpg(void *ptr, uint8_t order) {
	hn_set_pgblk_used(PGROUNDDOWN(ptr), MAD_ALLOC_KERNEL, order);
}

void hn_madpool_init(hn_madpool_t *madpool, pgaddr_t last, pgaddr_t next) {
	memset(madpool->descs, 0, sizeof(madpool->descs));
	madpool->last = last;
	madpool->next = next;
}

///
/// @brief Find a MAD with specified order which manages the address.
///
/// @param pool Target pool.
/// @param pgaddr Target paged address.
/// @param order Target order.
/// @return Found MAD. NULL if not found.
///
hn_mad_t *_hn_find_mad(hn_madpool_t *pool, uint32_t pgaddr, uint8_t order) {
	for (uint16_t i = 0; i < ARRAYLEN(pool->descs); ++i) {
		// There's no more MAD once we found that a MAD does not present.
		if (!(pool->descs[i].flags & MAD_P))
			break;
		if (ISINRANGE(pool->descs[i].pgaddr, MM_BLKPGSIZE(order), pgaddr)) {
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
void hn_set_pgblk_used(pgaddr_t pgaddr, uint8_t type, uint8_t order) {
	hn_pmad_t *pmad = hn_pmad_get(pgaddr);
	assert(pmad);

	// Lesser hint and greater hint.
	uint8_t lhint = order & LHINT, ghint = order & GHINT;
	order &= ~(LHINT | GHINT);

	pgaddr_t i = pmad->madpools[order];
	while (ISVALIDPG(i)) {
		pgaddr_t vaddr = hn_tmpmap(i, 1, PTE_P | PTE_RW);
		assert(ISVALIDPG(vaddr));

		hn_madpool_t *pool = UNPGADDR(vaddr);
		hn_mad_t *mad = _hn_find_mad(pool, pgaddr, order);
		if (mad) {
			pgaddr_t addr = mad->pgaddr;

			mad->flags = MAD_P;
			mad->type = type;
			++mad->ref_count;

			hn_tmpunmap(vaddr);

			if (order && lhint) {
				hn_set_pgblk_used(addr, type, LHINT | (order - 1));
				hn_set_pgblk_used(addr + MM_BLKPGSIZE(order), type, LHINT | (order - 1));
				return;
			}
			if (order < MM_MAXORD) {
				hn_set_pgblk_used(addr, type, GHINT | (order + 1));

				if (order && (!ghint)) {
					hn_set_pgblk_used(addr, type, LHINT | (order - 1));
					hn_set_pgblk_used(addr + MM_BLKPGSIZE(order) / 2, type, LHINT | (order - 1));
				}
			}
			return;
		}

		assert(i != pool->next);
		i = pool->next;
		hn_tmpunmap(vaddr);
	}

	assert(false);
}

void hn_set_pgblk_free(pgaddr_t addr, uint8_t order) {
	hn_pmad_t *const pmad = hn_pmad_get(addr);
	assert(pmad);

	pgaddr_t i = pmad->madpools[order];
	while (ISVALIDPG(i)) {
		pgaddr_t vaddr = hn_tmpmap(i, 1, PTE_P | PTE_RW);
		assert(ISVALIDPG(vaddr));

		hn_madpool_t *const pool = UNPGADDR(vaddr);
		hn_mad_t *mad = _hn_find_mad(pool, addr, order);
		if (mad) {
			if (!(--mad->ref_count)) {
				mad->type = MAD_ALLOC_FREE;

				if (order < MM_MAXORD) {
					hn_mad_t *nearest_mad = mad + ((mad - pool->descs) & 1 ? -1 : 1);
					assert(nearest_mad->flags & MAD_P);

					if (nearest_mad->type == MAD_ALLOC_FREE)
						hn_set_pgblk_free(addr, order + 1);
				}
			}
			hn_tmpunmap(vaddr);
			return;
		}

		assert(i != pool->next);
		i = pool->next;
		hn_tmpunmap(vaddr);
	}
}
