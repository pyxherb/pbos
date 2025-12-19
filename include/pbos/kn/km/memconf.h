#ifndef _PBOS_KN_KM_MEMCONF_H_
#define _PBOS_KN_KM_MEMCONF_H_

#include <pbos/common.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
	#include <pbos/kfxx/rbtree.hh>
#endif

PBOS_EXTERN_C_BEGIN

typedef uintptr_t (*hn_vpm_level_rounddowner_t)(uintptr_t addr);

typedef struct _kn_paging_config_t {
	size_t pgtab_level;
	hn_vpm_level_rounddowner_t *vpm_rounddowners;
	size_t *vpm_level_size;
#ifdef __cplusplus
	kfxx::rbtree_t<void *> *kspace_vpm_query_tree;
#else
	void *kspace_vpm_query_tree;
#endif
} kn_paging_config_t;

extern const kn_paging_config_t *kn_cur_paging_config;

PBOS_EXTERN_C_END

#endif
