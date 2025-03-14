#include <pbos/km/logger.h>
#include "../../mm.h"

static uintptr_t hn_level0_rounddowner(uintptr_t addr) {
	return addr & (uintptr_t)VADDR(PDX_MAX, 0, 0);
}

static uintptr_t hn_level1_rounddowner(uintptr_t addr) {
	return addr & (uintptr_t)VADDR(PDX_MAX, PTX_MAX, 0);
}

kf_rbtree_t hn_kspace_vpm_query_tree[HN_VPM_LEVEL_MAX + 1];
size_t hn_vpm_level_size[] = {
	(size_t)VADDR(1, 0, 0),
	(size_t)VADDR(0, 1, 0)
};
hn_vpm_level_rounddowner_t kn_mm_vpm_rounddowners[] = {
	hn_level0_rounddowner,
	hn_level1_rounddowner
};

kf_rbtree_t *kn_mm_get_vpm_lookup_tree(mm_context_t *context, const void *addr, int level) {
	return ISINRANGE(USPACE_VBASE, USPACE_SIZE, addr)
			   ? &context->uspace_vpm_query_tree[level]
			   : &hn_kspace_vpm_query_tree[level];
}

kn_mm_vpm_poolpg_t **kn_mm_get_vpm_pool_list(mm_context_t *context, const void *addr, int level) {
	return ISINRANGE(USPACE_VBASE, USPACE_SIZE, addr)
			   ? &context->uspace_vpm_poolpg_list
			   : &hn_kspace_vpm_poolpg_list;
}

