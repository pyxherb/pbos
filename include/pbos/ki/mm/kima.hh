#ifndef _PBOS_KI_KM_KIMA_H_
#define _PBOS_KI_KM_KIMA_H_

#include "misc.hh"

PBOS_EXTERN_C_BEGIN

typedef struct _kima_ublk_poolpg_t kima_ublk_poolpg_t;
typedef struct _kima_vpgdesc_poolpg_t kima_vpgdesc_poolpg_t;

typedef struct _kima_pool_t {
	kima_ublk_poolpg_t *ublk_poolpg_list = nullptr;
	kfxx::rbtree_t<void *> ublk_query_tree, ublk_free_tree;
	kima_vpgdesc_poolpg_t *vpgdesc_poolpg_list = nullptr;
	kfxx::rbtree_t<void *> vpgdesc_query_tree, vpgdesc_free_tree;
	size_t num_allocated_pages = 0;

	~_kima_pool_t();
} kima_pool_t;

extern kima_pool_t *mm_global_pool;

void ki_init_kima_pool(kima_pool_t *pool);

//
// User block definitions.
//

typedef struct _kima_ublk_t : public kfxx::rbtree_t<void *>::node_t {
	size_t size;
} kima_ublk_t;

typedef struct _kima_ublk_poolpg_t kima_ublk_poolpg_t;

typedef struct _kima_ublk_poolpg_header_t {
	kima_ublk_poolpg_t *prev, *next;
	size_t used_num;
} kima_ublk_poolpg_header_t;

typedef struct _kima_ublk_poolpg_t {
	kima_ublk_poolpg_header_t header;
	kima_ublk_t slots[(PAGESIZE - sizeof(kima_ublk_poolpg_header_t)) / sizeof(kima_ublk_t)];
} kima_ublk_poolpg_t;

kima_ublk_t* kima_lookup_ublk(kima_pool_t *pool, void* ptr);
kima_ublk_t* kima_lookup_nearest_ublk(kima_pool_t *pool, void* ptr);
kima_ublk_t *kima_alloc_ublk(kima_pool_t *pool, void *ptr, size_t size);
void kima_free_ublk(kima_pool_t *pool, kima_ublk_t *ublk);

//
// Virtual page descriptor definitions.
//

typedef struct _kima_vpgdesc_t : public kfxx::rbtree_t<void *>::node_t {
	size_t ref_count;
} kima_vpgdesc_t;

typedef struct _kima_vpgdesc_poolpg_t kima_vpgdesc_poolpg_t;

typedef struct _kima_vpgdesc_poolpg_header_t {
	kima_vpgdesc_poolpg_t *prev, *next;
	size_t used_num;
} kima_vpgdesc_poolpg_header_t;

typedef struct _kima_vpgdesc_poolpg_t {
	kima_vpgdesc_poolpg_header_t header;
	kima_vpgdesc_t slots[(PAGESIZE - sizeof(kima_vpgdesc_poolpg_header_t)) / sizeof(kima_vpgdesc_t)];
} kima_vpgdesc_poolpg_t;

kima_vpgdesc_t* kima_lookup_vpgdesc(kima_pool_t *pool, void* ptr);
kima_vpgdesc_t *kima_alloc_vpgdesc(kima_pool_t *pool, void *ptr);
void kima_free_vpgdesc(kima_pool_t *pool, kima_vpgdesc_t *vpgdesc);

void *kima_vpgalloc(void *addr, size_t size);
void kima_vpgfree(void *addr, size_t size);

//
// Common allocation APIs.
//

PBOS_NODISCARD void *kima_alloc(kima_pool_t *pool, size_t size, size_t alignment);
PBOS_NODISCARD void *kima_realloc(kima_pool_t *pool, void *old_ptr, size_t size, size_t alignment);
PBOS_NODISCARD void *kima_realloc_in_place(kima_pool_t *pool, void *old_ptr, size_t size, size_t alignment);
void kima_free(kima_pool_t *pool, void *ptr);

void kima_free_pool(kima_pool_t *pool);

size_t kima_get_allocated_page_num(kima_pool_t *pool);

PBOS_EXTERN_C_END

#endif
