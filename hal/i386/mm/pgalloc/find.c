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

		if ((addr >= hn_pmad_list[i].attribs.base) &&
			(addr <= (hn_pmad_list[i].attribs.base + (hn_pmad_list[i].attribs.len - 1))))
			return &(hn_pmad_list[i]);
	}

	return NULL;
}

pgaddr_t hn_alloc_freeblk_in_area(hn_pmad_t *area) {
	hn_madpool_t *pool = area->madpools;

	while (pool) {
		for (uint16_t i = 0; i < ARRAYLEN(pool->descs); ++i) {
			hn_mad_t *mad = &(pool->descs[i]);
			// There's no more MAD once we found that a MAD does not present.
			if (!(mad->flags & MAD_P))
				break;
			// kprintf("PGADDR: %p, type = %d\n", UNPGADDR(mad->pgaddr), (int)mad->type);
			if (mad->type == MAD_ALLOC_FREE) {
				pgaddr_t pgaddr = mad->pgaddr;
				return pgaddr;
			}
		}

		assert(pool != pool->next);
		pool = pool->next;
	}

	return NULLPG;
}

pgaddr_t hn_alloc_freeblk(uint8_t type) {
	PMAD_FOREACH(i) {
		if (i->attribs.type != type)
			continue;
		pgaddr_t addr = hn_alloc_freeblk_in_area(i);
		if (ISVALIDPG(addr))
			return addr;
	}

	return NULLPG;
}
