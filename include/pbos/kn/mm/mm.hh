#ifndef _PBOS_KN_KM_MM_H_
#define _PBOS_KN_KM_MM_H_

#include "memconf.h"

#include <pbos/mm/mm.h>
#include <pbos/km/panic.h>
#include <pbos/kfxx/rbtree.hh>

#include <pbos/generated/km.h>
#include KN_ARCH_MEMCONF_HEADER_PATH

// #include <arch/i386/paging.h>

PBOS_EXTERN_C_BEGIN

#define KM_MM_VPM_ALLOC 0x00000001

extern mm_context_t **mm_cur_contexts;

void kn_mm_init_kima();

void kn_mm_sync_global_mappings(const mm_context_t *src);
void kn_mm_copy_global_mappings(mm_context_t *dest, const mm_context_t *src);

void *kn_rounddown_to_page_leveled_addr(const void *const addr, int level);

PBOS_EXTERN_C_END

#endif
