#ifndef _HAL_I386_MM_H_
#define _HAL_I386_MM_H_

#include <arch/i386/kargs.h>
#include <arch/i386/paging.h>
#include <pbos/kf/rbtree.h>
#include <pbos/km/assert.h>
#include <pbos/kn/km/mm.h>
#include <stdalign.h>
#include <string.h>
#include "mm/kima/init.h"
#include "mm/misc.h"
#include "mm/vmmgr/vm.h"

#define HN_MAX_PGTAB_LEVEL 2

#define HN_VPM_ALLOC 0x00000001

typedef uint32_t hn_vpm_flags_t;

typedef struct _hn_vpm_t {
	kf_rbtree_node_t node_header;
	void *addr;
	size_t ref_count;
	hn_vpm_flags_t flags;
} hn_vpm_t;

typedef struct _hn_vpm_poolpg_t hn_vpm_poolpg_t;

typedef struct _hn_vpm_poolpg_header_t {
	hn_vpm_poolpg_t *prev, *next;
	size_t used_num;
} hn_vpm_poolpg_header_t;

typedef struct _hn_vpm_poolpg_t {
	hn_vpm_poolpg_header_t header;
	hn_vpm_t descs[(PAGESIZE - sizeof(hn_vpm_poolpg_header_t)) / sizeof(hn_vpm_t)];
} hn_vpm_poolpg_t;

extern hn_vpm_poolpg_t *hn_kspace_vpm_poolpg_list;
extern kf_rbtree_t hn_kspace_vpm_query_tree;

typedef struct _mm_context_t {
	arch_pde_t *pdt;
	hn_vpm_poolpg_t *uspace_vpm_poolpg_list;
	kf_rbtree_t uspace_vpm_query_tree;
} mm_context_t;

bool hn_vpm_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y);
void hn_vpm_nodefree(kf_rbtree_node_t *p);

void mm_sync_global_mappings(const mm_context_t *src);
void mm_copy_global_mappings(mm_context_t *dest, const mm_context_t *src);

hn_vpm_t *hn_mm_lookup_vpm(mm_context_t *context, const void* addr);
hn_vpm_t *hn_mm_alloc_vpm_slot(mm_context_t *context, const void *addr);
km_result_t hn_mm_insert_vpm(mm_context_t *context, const void *addr);
km_result_t hn_mm_insert_vpm_unchecked(mm_context_t *context, const void *const addr);
void hn_mm_free_vpm(mm_context_t *context, const void *addr);
void hn_mm_init();

#endif
