#ifndef _PBOS_KN_KM_MM_H_
#define _PBOS_KN_KM_MM_H_

#include <pbos/km/mm.h>
#include <pbos/km/panic.h>
#include <pbos/kf/rbtree.h>

#include <arch/i386/paging.h>

#define KM_MM_VPM_ALLOC 0x00000001

typedef uint32_t kn_mm_vpm_flags_t;

typedef struct _hn_vpm_t {
	kf_rbtree_node_t node_header;
	void *addr;
	union {
		size_t subref_count;
		void *map_addr;
	};
	kn_mm_vpm_flags_t flags;
} hn_vpm_t;

typedef struct _kn_mm_vpm_poolpg_t kn_mm_vpm_poolpg_t;

typedef struct _hn_vpm_poolpg_header_t {
	kn_mm_vpm_poolpg_t *prev, *next;
	size_t used_num;
} hn_vpm_poolpg_header_t;

typedef struct _kn_mm_vpm_poolpg_t {
	hn_vpm_poolpg_header_t header;
	hn_vpm_t descs[(PAGESIZE - sizeof(hn_vpm_poolpg_header_t)) / sizeof(hn_vpm_t)];
} kn_mm_vpm_poolpg_t;

typedef uintptr_t (*hn_vpm_level_rounddowner_t)(uintptr_t addr);
extern hn_vpm_level_rounddowner_t kn_mm_vpm_rounddowners[];
extern size_t hn_vpm_level_size[];

void kn_mm_sync_global_mappings(const mm_context_t *src);
void kn_mm_copy_global_mappings(mm_context_t *dest, const mm_context_t *src);

void *kn_rounddown_to_page_leveled_addr(const void *const addr, int level);

kf_rbtree_t *kn_mm_get_vpm_lookup_tree(mm_context_t *context, const void *addr, int level);
kn_mm_vpm_poolpg_t **kn_mm_get_vpm_pool_list(mm_context_t *context, const void *addr, int level);

bool kn_vpm_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y);
void kn_vpm_nodefree(kf_rbtree_node_t *p);
hn_vpm_t *kn_mm_lookup_vpm(mm_context_t *context, const void* addr, int level);
hn_vpm_t *kn_mm_alloc_vpm_slot(mm_context_t *context, const void *addr, int level);
km_result_t kn_mm_insert_vpm(mm_context_t *context, const void *addr);
km_result_t kn_mm_insert_vpm_unchecked(mm_context_t *context, const void *const addr, int level);
void kn_mm_free_vpm(mm_context_t *context, const void *addr);
void kn_mm_free_vpm_unchecked(mm_context_t *context, const void *addr, int level);

void *kn_lookup_pgdir_mapped_addr(void *addr);
void *kn_lookup_pgdir(mm_context_t *ctxt, void *addr, int level);
void *kn_mm_alloc_pgdir(mm_context_t *ctxt, void *addr, int level);
void kn_mm_free_pgdir(mm_context_t *ctxt, void *addr, int level);

#endif
