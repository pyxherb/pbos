#include <pbos/km/logger.h>
#include "../../mm.hh"

PBOS_EXTERN_C_BEGIN

static uintptr_t hn_level0_rounddowner(uintptr_t addr) {
	return (uintptr_t)VADDR(PDX(addr), 0, 0);
}

static uintptr_t hn_level1_rounddowner(uintptr_t addr) {
	return (uintptr_t)VADDR(PDX(addr), PTX(addr), 0);
}

static kfxx::rbtree_t<void *> hn_kspace_vpm_query_tree[HN_VPM_LEVEL_MAX + 1];

const kn_paging_config_t KN_PAGING_CONFIG_32BIT = {
	.pgtab_level = HN_VPM_LEVEL_MAX + 1,

	.vpm_rounddowners = (hn_vpm_level_rounddowner_t[]){
		hn_level0_rounddowner,
		hn_level1_rounddowner },
	.vpm_level_size = (size_t[]){ (size_t)VADDR(1, 0, 0), (size_t)VADDR(0, 1, 0) },
	.kspace_vpm_query_tree = hn_kspace_vpm_query_tree
};

kfxx::rbtree_t<void *> *kn_mm_get_vpm_lookup_tree(mm_context_t *context, const void *addr, int level) {
	return !mm_probe_user_space(context, addr, 0)
			   ? &context->uspace_vpm_query_tree[level]
			   : &kn_cur_paging_config->kspace_vpm_query_tree[level];
}

kn_mm_vpm_poolpg_t **kn_mm_get_vpm_pool_list(mm_context_t *context, const void *addr, int level) {
	return !mm_probe_user_space(context, addr, 0)
			   ? &context->uspace_vpm_poolpg_list
			   : &hn_kspace_vpm_poolpg_list;
}

PBOS_EXTERN_C_END
