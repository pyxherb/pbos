#ifndef _HAL_I386_MM_HH_
#define _HAL_I386_MM_HH_

#include <pbos/kfxx/rbtree.hh>
#include "mm.h"

PBOS_EXTERN_C_BEGIN

typedef struct _mm_context_t {
	arch_pde_t *pdt;
	kn_mm_vpm_poolpg_t *uspace_vpm_poolpg_list;
	kfxx::rbtree_t<void *> uspace_vpm_query_tree[HN_VPM_LEVEL_MAX + 1];
} mm_context_t;

extern kn_mm_vpm_poolpg_t *hn_kspace_vpm_poolpg_list;
extern kfxx::rbtree_t<void *> hn_kspace_vpm_query_tree[HN_VPM_LEVEL_MAX + 1];

extern size_t hn_vpm_level_size[HN_VPM_LEVEL_MAX + 1];

PBOS_EXTERN_C_END

#endif
