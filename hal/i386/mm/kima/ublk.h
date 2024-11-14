#ifndef _KIMA_UBLK_H_
#define _KIMA_UBLK_H_

#include "vmalloc.h"
#include <pbos/kf/rbtree.h>

typedef struct _kima_ublk_t {
	kf_rbtree_node_t node_header;
	void *ptr;
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

extern kima_ublk_poolpg_t *kima_ublk_poolpg_list;
extern kf_rbtree_t kima_ublk_query_tree;

bool kima_ublk_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y);
void kima_ublk_nodefree(kf_rbtree_node_t *p);

kima_ublk_t *kima_lookup_ublk(void *ptr);
kima_ublk_t *kima_lookup_nearest_ublk(void *ptr);
kima_ublk_t *kima_alloc_ublk(void *ptr, size_t size);
void kima_free_ublk(kima_ublk_t *ublk);

#endif
