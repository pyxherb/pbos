#ifndef _KIMA_UBLK_H_
#define _KIMA_UBLK_H_

#include <pbos/kfxx/rbtree.hh>
#include "vmalloc.hh"

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
	kima_ublk_t slots[(DEFAULT_PAGESIZE - sizeof(kima_ublk_poolpg_header_t)) / sizeof(kima_ublk_t)];
} kima_ublk_poolpg_t;

extern kima_ublk_poolpg_t *kima_ublk_poolpg_list;
extern kfxx::rbtree_t<void *> kima_ublk_query_tree, kima_ublk_free_tree;

PBOS_FORCEINLINE void kima_ublk_free(kima_ublk_t* p) {
	p->rb_value = NULL;
}

PBOS_FORCEINLINE kima_ublk_t* kima_lookup_ublk(void* ptr) {
	kfxx::rbtree_t<void*>::node_t* node = kima_ublk_query_tree.find(ptr);

	if (!node)
		return nullptr;

	return static_cast<kima_ublk_t*>(node);
}

PBOS_FORCEINLINE kima_ublk_t* kima_lookup_nearest_ublk(void* ptr) {
	kfxx::rbtree_t<void*>::node_t* node = kima_ublk_query_tree.find_max_lteq(ptr);

	if (!node)
		return NULL;

	return static_cast<kima_ublk_t*>(node);
}

kima_ublk_t *kima_alloc_ublk(void *ptr, size_t size);
void kima_free_ublk(kima_ublk_t *ublk);

#endif
