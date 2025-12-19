#ifndef _HAL_I386_MM_HH_
#define _HAL_I386_MM_HH_

#include <pbos/kfxx/rbtree.hh>
#include "mm.h"

PBOS_EXTERN_C_BEGIN

typedef struct _mm_context_t {
	arch_pde_t *pdt;
	kn_mm_vpm_poolpg_t *uspace_vpm_poolpg_list = nullptr;
	kfxx::rbtree_t<void *> *uspace_vpm_query_tree = nullptr;
} mm_context_t;

extern kn_mm_vpm_poolpg_t *hn_kspace_vpm_poolpg_list;

PBOS_EXTERN_C_END

#endif
