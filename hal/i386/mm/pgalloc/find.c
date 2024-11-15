#include <hal/i386/mm.h>

///
/// @brief Get PMAD by a physical address.
///
/// @param addr Physical address to find.
/// @return Corresponding PMAD. NULL if not found.
///
hn_pmad_t *hn_pmad_get(pgaddr_t addr) {
	for (uint8_t i = 0; i < ARRAYLEN(hn_pmad_list); ++i) {
		if (hn_pmad_list[i].attribs.type == KN_PMEM_END)
			break;

		if ((hn_pmad_list[i].attribs.base > addr) ||
			(hn_pmad_list[i].attribs.base + (hn_pmad_list[i].attribs.len - 1) < addr))
			continue;

		return &(hn_pmad_list[i]);
	}

	return NULL;
}

pgaddr_t hn_alloc_freeblk_in_area(hn_pmad_t *area, uint8_t order) {
	pgaddr_t k = area->madpools[order];

	while (ISVALIDPG(k)) {
		pgaddr_t vaddr = hn_tmpmap(k, 1, PTE_P);
		assert(ISVALIDPG(vaddr));

		hn_madpool_t *pool = UNPGADDR(vaddr);
		for (uint16_t i = 0; i < ARRAYLEN(pool->descs); ++i) {
			hn_mad_t *mad = &(pool->descs[i]);
			// There's no more MAD once we found that a MAD does not present.
			if (!(mad->flags & MAD_P))
				break;
			// kprintf("PGADDR: %p, type = %d\n", UNPGADDR(mad->pgaddr), (int)mad->type);
			if (mad->type == MAD_ALLOC_FREE) {
				pgaddr_t pgaddr = mad->pgaddr;
				hn_tmpunmap(vaddr);
				return pgaddr;
			}
		}

		assert(k != pool->next);
		k = pool->next;
		hn_tmpunmap(vaddr);
	}

	return NULLPG;
}

pgaddr_t hn_alloc_freeblk(uint8_t type, uint8_t order) {
	PMAD_FOREACH(i) {
		if (i->attribs.type != type)
			continue;
		pgaddr_t addr = hn_alloc_freeblk_in_area(i, order);
		if (ISVALIDPG(addr))
			return addr;
	}

	return NULLPG;
}
