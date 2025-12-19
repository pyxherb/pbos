#ifndef _PBOS_KN_KM_MM_H_
#define _PBOS_KN_KM_MM_H_

#include "memconf.h"

#include <pbos/km/mm.h>
#include <pbos/km/panic.h>
#include <pbos/kfxx/rbtree.hh>

#include <arch/i386/paging.h>

PBOS_EXTERN_C_BEGIN

#define KM_MM_VPM_ALLOC 0x00000001

typedef uint32_t kn_mm_vpm_flags_t;

struct hn_vpm_t : public kfxx::rbtree_t<void *>::node_t {
	union {
		size_t subref_count;
		void *map_addr;
	};
	kn_mm_vpm_flags_t flags;
};

typedef struct _kn_mm_vpm_poolpg_t kn_mm_vpm_poolpg_t;

typedef struct _hn_vpm_poolpg_header_t {
	kn_mm_vpm_poolpg_t *prev, *next;
	size_t used_num;
} hn_vpm_poolpg_header_t;

typedef struct _kn_mm_vpm_poolpg_t {
	hn_vpm_poolpg_header_t header;
	hn_vpm_t descs[(PAGESIZE - sizeof(hn_vpm_poolpg_header_t)) / sizeof(hn_vpm_t)];
} kn_mm_vpm_poolpg_t;

void kn_mm_init_kima();

void kn_mm_sync_global_mappings(const mm_context_t *src);
void kn_mm_copy_global_mappings(mm_context_t *dest, const mm_context_t *src);

void *kn_rounddown_to_page_leveled_addr(const void *const addr, int level);

kfxx::rbtree_t<void *> *kn_mm_get_vpm_lookup_tree(mm_context_t *context, const void *addr, int level);
kn_mm_vpm_poolpg_t **kn_mm_get_vpm_pool_list(mm_context_t *context, const void *addr, int level);

void kn_vpm_nodefree(hn_vpm_t *p);
hn_vpm_t *kn_mm_lookup_vpm(mm_context_t *context, const void* addr, int level);
hn_vpm_t *kn_mm_alloc_vpm_slot(mm_context_t *context, const void *addr, int level);
km_result_t kn_mm_insert_vpm(mm_context_t *context, const void *addr);
km_result_t kn_mm_insert_vpm_unchecked(mm_context_t *context, const void *const addr, int level);
void kn_mm_free_vpm(mm_context_t *context, const void *addr);
void kn_mm_free_vpm_unchecked(mm_context_t *context, const void *addr, int level);

void *kn_lookup_pgdir_mapped_addr(void *addr);
void *kn_lookup_pgdir(mm_context_t *ctxt, void *addr, int level);
PBOS_NODISCARD void *kn_mm_alloc_pgdir(mm_context_t *ctxt, void *addr, int level);
void kn_mm_free_pgdir(mm_context_t *ctxt, void *addr, int level);

PBOS_EXTERN_C_END

#endif
