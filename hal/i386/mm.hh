#ifndef _HAL_I386_MM_HH_
#define _HAL_I386_MM_HH_

#include <pbos/kfxx/rbtree.hh>
#include "mm.h"

typedef struct _mm_context_t {
	arch_pde_t *pdt;
	kn_mm_vpm_poolpg_t *uspace_vpm_poolpg_list;
	kf_rbtree_t uspace_vpm_query_tree[HN_VPM_LEVEL_MAX + 1];
} mm_context_t;

#endif
