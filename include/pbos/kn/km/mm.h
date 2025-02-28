#ifndef _PBOS_KN_KM_MM_H_
#define _PBOS_KN_KM_MM_H_

#include <pbos/km/mm.h>
#include <pbos/km/panic.h>
#include <pbos/kf/rbtree.h>

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

typedef uintptr_t (*hn_vpm_level_rounddowner_t)(uintptr_t addr);
extern size_t kn_mm_vpm_level_max;
extern hn_vpm_level_rounddowner_t kn_mm_vpm_rounddowners[];
extern size_t hn_vpm_level_size[];

void kn_mm_sync_global_mappings(const mm_context_t *src);
void kn_mm_copy_global_mappings(mm_context_t *dest, const mm_context_t *src);

void *kn_lookup_pgdir(mm_context_t *ctxt, void *addr, int level);
void *kn_mm_alloc_pgdir(mm_context_t *ctxt, void *addr, int level);
void kn_mm_free_pgdir(mm_context_t *ctxt, void *addr, int level);

#endif
