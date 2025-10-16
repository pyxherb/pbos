#ifndef _HAL_I386_MM_H_
#define _HAL_I386_MM_H_

#include <arch/i386/kargs.h>
#include <arch/i386/paging.h>
#include <pbos/kf/rbtree.h>
#include <pbos/km/assert.h>
#include <pbos/kn/km/mm.h>
#include <stdalign.h>
#include <string.h>
#include "mm/misc.h"
#include "mm/vmmgr/vm.h"

PBOS_EXTERN_C_BEGIN

#define HN_MAX_PGTAB_LEVEL 2

#define HN_VPM_LEVEL_MAX 1

enum {
	HN_MM_INIT_STAGE_INITIAL = 0,
	HN_MM_INIT_STAGE_AREAS_INITIAL,
	HN_MM_INIT_STAGE_INITIAL_AREAS_INITED,
	HN_MM_INIT_STAGE_AREAS_INITED
};

extern uint8_t hn_mm_init_stage;

extern kn_mm_vpm_poolpg_t *hn_kspace_vpm_poolpg_list;
extern kf_rbtree_t hn_kspace_vpm_query_tree[HN_VPM_LEVEL_MAX + 1];

typedef struct _mm_context_t {
	arch_pde_t *pdt;
	kn_mm_vpm_poolpg_t *uspace_vpm_poolpg_list;
	kf_rbtree_t uspace_vpm_query_tree[HN_VPM_LEVEL_MAX + 1];
} mm_context_t;

extern size_t hn_vpm_level_size[HN_VPM_LEVEL_MAX + 1];

void hn_mm_init();

PBOS_EXTERN_C_END

#endif
