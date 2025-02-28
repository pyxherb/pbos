#include "pgalloc.h"
#include <hal/i386/mm.h>
#include <math.h>
#include <pbos/km/logger.h>

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

bool hn_mad_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y) {
	const hn_mad_t *_x = (const hn_mad_t *)x,
				   *_y = (const hn_mad_t *)y;

	return _x->pgaddr < _y->pgaddr;
}

void hn_mad_nodefree(kf_rbtree_node_t *p) {
}

hn_mad_t *hn_get_mad(pgaddr_t pgaddr) {
	// kprintf("hn_set_pgblk_used: %p (%d)\n", UNPGADDR(pgaddr), (int)order & 0x3f);
	hn_pmad_t *pmad = hn_pmad_get(pgaddr);
	if(!pmad)
		km_panic("No PMAD corresponds to physical address %p", UNPGADDR(pgaddr));

	hn_mad_t query_mad = {
		.pgaddr = pgaddr
	};

	kf_rbtree_node_t *mad_node = kf_rbtree_find(&pmad->mad_query_tree, &query_mad.node_header);
	if (!mad_node) {
		km_panic("Physical memory block not found: %p", UNPGADDR(pgaddr));
	}

	hn_mad_t *mad = PB_CONTAINER_OF(hn_mad_t, node_header, mad_node);

	return mad;
}

///
/// @brief Mark a page as allocated.
///
/// @param pgaddr Paged address to mark.
/// @param type Allocation type.
///
void hn_set_pgblk_used(pgaddr_t pgaddr, uint8_t type) {
	hn_mad_t *mad = hn_get_mad(pgaddr);

	mad->flags = MAD_P;
	mad->type = type;
	++mad->ref_count;

	return;
}

void hn_set_pgblk_free(pgaddr_t addr) {
	hn_mad_t *mad = hn_get_mad(addr);

	kd_assert(mad->ref_count);
	if (!(--mad->ref_count)) {
		mad->flags = MAD_P;
		mad->type = MAD_ALLOC_FREE;
	}
	return;
}

void *kn_lookup_pgdir_mapped_addr(void *addr) {
	hn_mad_t *mad = hn_get_mad(PGROUNDDOWN(addr));

	return UNPGADDR(mad->exdata.mapped_pgtab_addr);
}
