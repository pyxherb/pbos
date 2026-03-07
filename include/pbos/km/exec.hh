#ifndef _PBOS_KM_EXEC_HH_
#define _PBOS_KM_EXEC_HH_

#include <pbos/generated/km.h>
#include <pbos/kfxx/rbtree.hh>
#include <pbos/kfxx/uuid.hh>
#include "exec.h"
#include KN_ARCH_MEMCONF_HEADER_PATH

PBOS_EXTERN_C_BEGIN

typedef struct _kn_binldr_registry_t : public kfxx::rbtree_t<kf_uuid_t>::node_t {
	km_binldr_t binldr;
} kn_binldr_registry_t;

typedef struct _km_binseg_page_desc_t : public kfxx::rbtree_t<void *>::node_t {
} km_binseg_page_desc_t;

typedef struct alignas(PAGESIZE) _km_binseg_page_pool_t {
	_km_binseg_page_pool_t *prev = nullptr, *next = nullptr;
	size_t used_num = 0;
	km_binseg_page_desc_t descs[(PAGESIZE - sizeof(void *) - sizeof(void *) - sizeof(size_t)) / sizeof(km_binseg_page_desc_t)];
} km_binseg_page_pool_t;

static_assert(sizeof(km_binseg_page_pool_t) == PAGESIZE);

typedef struct _km_binseg_t {
	_km_binseg_t *prev, *next;
	void *vaddr_base;
	size_t size;
	size_t cur_offset;
	km_binseg_page_pool_t *pages = nullptr, *cur_page_pool = nullptr;
	kfxx::rbtree_t<void *> page_descs_query_tree;
	mm_pgaccess_t access;
} km_binseg_t;

typedef struct _km_binproto_t : public kfxx::rbtree_t<fs_fcb_t *>::node_t {
	km_binseg_t *segments = nullptr;
} km_binproto_t;

extern kfxx::rbtree_t<kf_uuid_t> kn_registered_binldrs;

PBOS_EXTERN_C_END

#endif
