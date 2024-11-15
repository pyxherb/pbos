#ifndef _HAL_I386_MM_H_
#define _HAL_I386_MM_H_

#include <arch/i386/kargs.h>
#include <arch/i386/paging.h>
#include <pbos/km/assert.h>
#include <pbos/km/mm.h>
#include <pbos/kf/rbtree.h>
#include <stdalign.h>
#include <string.h>
#include "mm/misc.h"
#include "mm/kima/init.h"
#include "mm/vmmgr/vm.h"

#define HN_MAX_PGTAB_LEVEL 2

#define MM_VPD_ALLOC 0x01

typedef uint8_t mm_vpd_flags_t;

/// @brief Virtual Page Descriptor (VPD)
typedef struct _mm_vpd_t {
	kf_rbtree_node_t node_header;
	void *addr;
	mm_vpd_flags_t flags;
} mm_vpd_t;

typedef struct _mm_vpdpool_t mm_vpdpool_t;

typedef struct _mm_vpdpool_header_t {
	mm_vpdpool_t *prev, *next;
	size_t used_num;
} mm_vpdpool_header_t;

typedef struct _mm_vpdpool_t {
	mm_vpdpool_header_t header;
	mm_vpd_t descs[(PAGESIZE - sizeof(mm_vpdpool_header_t)) / sizeof(mm_vpd_t)];
} mm_vpdpool_t;

typedef struct _mm_context_t {
	arch_pde_t *pdt;
	mm_vpdpool_t *vpd_pools;
	kf_rbtree_t vpd_rbtree;
} mm_context_t;

bool hn_vpd_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y) ;
void hn_vpd_nodefree(kf_rbtree_node_t *p);

void mm_sync_global_mappings(const mm_context_t *src);
void mm_copy_global_mappings(mm_context_t *dest, const mm_context_t *src);

mm_vpd_t *hn_mm_lookup_vpd(mm_context_t *context, const void* addr);
mm_vpd_t *hn_mm_alloc_vpd_slot(mm_context_t *context);
km_result_t hn_mm_insert_vpd(mm_context_t *context, const void *addr);
km_result_t hn_mm_insert_vpd_unchecked(mm_context_t *context, const void *addr);
void hn_mm_free_vpd(mm_context_t *context, const void *addr);
void hn_mm_init();

#endif
