#include <hal/i386/mm.h>

///
/// @brief Get PMAD by a physical address.
///
/// @param addr Physical address to find.
/// @return Corresponding PMAD. NULL if not found.
///
hn_pmad_t *hn_pmad_get(pgaddr_t addr) {
	for (uint8_t i = 0; i < PB_ARRAYSIZE(hn_pmad_list); ++i) {
		if (hn_pmad_list[i].attribs.type == KN_PMEM_END)
			break;

		if ((addr >= hn_pmad_list[i].attribs.base) &&
			(addr <= (hn_pmad_list[i].attribs.base + (hn_pmad_list[i].attribs.len - 1))))
			return &(hn_pmad_list[i]);
	}

	return NULL;
}

pgaddr_t hn_alloc_freeblk_in_area(hn_pmad_t *area) {
	kf_rbtree_foreach(i, &area->mad_query_tree) {
		hn_mad_t *mad = PB_CONTAINER_OF(hn_mad_t, node_header, i);
		assert(mad->flags & MAD_P);
		if (mad->type == MAD_ALLOC_FREE) {
			pgaddr_t pgaddr = mad->pgaddr;
			return pgaddr;
		}
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
