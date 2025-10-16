#ifndef _KIMA_VPGDESC_H_
#define _KIMA_VPGDESC_H_

#include <pbos/kfxx/rbtree.hh>
#include "vmalloc.hh"

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

extern kima_vpgdesc_poolpg_t *kima_vpgdesc_poolpg_list;
extern kfxx::rbtree_t<void *> kima_vpgdesc_query_tree, kima_vpgdesc_free_tree;

PBOS_FORCEINLINE void kima_vpgdesc_free(kima_vpgdesc_t* p) {
	p->rb_value = NULL;
}

kima_vpgdesc_t *kima_lookup_vpgdesc(void *ptr);
kima_vpgdesc_t *kima_alloc_vpgdesc(void *ptr);
void kima_free_vpgdesc(kima_vpgdesc_t *vpgdesc);

#endif
