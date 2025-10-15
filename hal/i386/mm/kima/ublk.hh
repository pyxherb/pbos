#ifndef _KIMA_UBLK_H_
#define _KIMA_UBLK_H_

#include <pbos/kfxx/rbtree.hh>
#include "vmalloc.hh"

#ifdef __cplusplus
typedef struct _kima_ublk_t {
	kfxx::RBTree<void *>::Node node_header;
	size_t size;
} kima_ublk_t;
#else
typedef struct _kima_ublk_t kima_ublk_t;
#endif

typedef struct _kima_ublk_poolpg_t kima_ublk_poolpg_t;

typedef struct _kima_ublk_poolpg_header_t {
	kima_ublk_poolpg_t *prev, *next;
	size_t used_num;
} kima_ublk_poolpg_header_t;

typedef struct _kima_ublk_poolpg_t {
	kima_ublk_poolpg_header_t header;
	kima_ublk_t slots[(PAGESIZE - sizeof(kima_ublk_poolpg_header_t)) / sizeof(kima_ublk_t)];
} kima_ublk_poolpg_t;

extern kima_ublk_poolpg_t *kima_ublk_poolpg_list;
extern kfxx::RBTree<void *> kima_ublk_query_tree, kima_ublk_free_tree;

void kima_ublk_free(kima_ublk_t *p);

kima_ublk_t *kima_lookup_ublk(void *ptr);
kima_ublk_t *kima_lookup_nearest_ublk(void *ptr);
kima_ublk_t *kima_alloc_ublk(void *ptr, size_t size);
void kima_free_ublk(kima_ublk_t *ublk);

#endif
