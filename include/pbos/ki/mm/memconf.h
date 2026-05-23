#ifndef _PBOS_KI_KM_MEMCONF_H_
#define _PBOS_KI_KM_MEMCONF_H_

#include <pbos/common.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
	#include <pbos/kfxx/rbtree.hh>
#endif

PBOS_EXTERN_C_BEGIN

typedef uintptr_t (*hali_pglevel_addr_rounddowner_t)(uintptr_t addr);

typedef struct _ki_paging_config_t {
	size_t pgtab_level;
	hali_pglevel_addr_rounddowner_t *addr_rounddowners;
	size_t *page_level_size;
} ki_paging_config_t;

extern const ki_paging_config_t *ki_cur_paging_config;

PBOS_EXTERN_C_END

#endif
