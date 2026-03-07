#ifndef _PBOS_KN_KM_MEMCONF_H_
#define _PBOS_KN_KM_MEMCONF_H_

#include <pbos/common.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
	#include <pbos/kfxx/rbtree.hh>
#endif

PBOS_EXTERN_C_BEGIN

typedef uintptr_t (*hn_pglevel_addr_rounddowner_t)(uintptr_t addr);

typedef struct _kn_paging_config_t {
	size_t pgtab_level;
	hn_pglevel_addr_rounddowner_t *addr_rounddowners;
	size_t *page_level_size;
} kn_paging_config_t;

extern const kn_paging_config_t *kn_cur_paging_config;

PBOS_EXTERN_C_END

#endif
