#ifndef _KIMA_VPGDESC_H_
#define _KIMA_VPGDESC_H_

#include <pbos/kfxx/rbtree.hh>
#include "vmalloc.hh"

#ifdef __cplusplus
typedef struct _kima_vpgdesc_t {
	kfxx::RBTree<void *>::Node node_header;
	size_t ref_count;
} kima_vpgdesc_t;
#else
typedef struct _kima_vpgdesc_t kima_vpgdesc_t;
#endif

typedef struct _kima_vpgdesc_poolpg_t kima_vpgdesc_poolpg_t;

typedef struct _kima_vpgdesc_poolpg_header_t {
	kima_vpgdesc_poolpg_t *prev, *next;
	size_t used_num;
} kima_vpgdesc_poolpg_header_t;

typedef struct _kima_vpgdesc_poolpg_t {
	kima_vpgdesc_poolpg_header_t header;
	kima_vpgdesc_t slots[(PAGESIZE - sizeof(kima_vpgdesc_poolpg_header_t)) / sizeof(kima_vpgdesc_t)];
} kima_vpgdesc_poolpg_t;

extern kima_vpgdesc_poolpg_t *kima_vpgdesc_poolpg_list;
extern kfxx::RBTree<void *> kima_vpgdesc_query_tree, kima_vpgdesc_free_tree;

void kima_vpgdesc_free(kima_vpgdesc_t *p);

kima_vpgdesc_t *kima_lookup_vpgdesc(void *ptr);
kima_vpgdesc_t *kima_alloc_vpgdesc(void *ptr);
void kima_free_vpgdesc(kima_vpgdesc_t *vpgdesc);

#endif
