#ifndef _PBOS_KI_KM_MM_H_
#define _PBOS_KI_KM_MM_H_

#include "memconf.h"

#include <pbos/mm/mm.h>
#include <pbos/km/panic.h>
#include <pbos/kfxx/rbtree.hh>

#include <pbos/generated/km.h>
#include KI_ARCH_MEMCONF_HEADER_PATH

PBOS_EXTERN_C_BEGIN

extern mm_context_t **mm_cur_contexts;

void ki_mm_init_global_allocator();

void kh_mm_sync_global_mappings(const mm_context_t *src);
void kh_mm_copy_global_mappings(mm_context_t *dest, const mm_context_t *src);

PBOS_EXTERN_C_END

#endif
