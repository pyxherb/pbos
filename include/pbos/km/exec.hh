#ifndef _PBOS_KM_EXEC_HH_
#define _PBOS_KM_EXEC_HH_

#include <pbos/kfxx/rbtree.hh>
#include <pbos/kfxx/uuid.hh>
#include "exec.h"
#include <pbos/generated/km.h>
#include KN_ARCH_MEMCONF_HEADER_PATH

PBOS_EXTERN_C_BEGIN

typedef struct _kn_binldr_registry_t : public kfxx::rbtree_t<kf_uuid_t>::node_t {
	km_binldr_t binldr;
} kn_binldr_registry_t;

typedef struct _km_binseg_page_desc_t {
	void *paddr;
} km_binseg_page_desc_t;

typedef struct _km_binseg_page_pool_t {
	_km_binseg_page_pool_t *prev, *next;
	size_t used_num;
	km_binseg_page_desc_t descs[DEFAULT_PAGESIZE / sizeof(km_binseg_page_desc_t)];
} km_binseg_page_pool_t;

typedef struct _km_binseg_t : public kfxx::rbtree_t<void *>::node_t {
	km_binseg_page_pool_t *pages;
	mm_pgaccess_t access;
} km_binseg_t;

typedef struct _km_binproto_t {
} km_binproto_t;

extern kfxx::rbtree_t<kf_uuid_t> kn_registered_binldrs;

PBOS_EXTERN_C_END

#endif
