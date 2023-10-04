#ifndef _OICOS_KF_RBTREE_H_
#define _OICOS_KF_RBTREE_H_

#include <oicos/km/assert.h>
#include <oicos/km/panic.h>
#include <oicos/km/result.h>
#include <stdbool.h>
#include <stdint.h>

#define KF_RBTREE_BLACK 0
#define KF_RBTREE_RED 1

#define kf_rbtree_parent(n) ((kf_rbtree_node_t *)(((size_t)n->p_color) & ~0x3))
#define kf_rbtree_setparent(n, _p) \
	n->p_color =                   \
		(kf_rbtree_node_t *)((((size_t)n->p_color) & 0x3) | (size_t)_p)

#define kf_rbtree_color(n) ((bool)(((size_t)n->p_color) & 0x3))
#define kf_rbtree_setcolor(n, c) \
	n->p_color = (kf_rbtree_node_t *)((((size_t)n->p_color) & ~0x3) | c)

#define kf_rbtree_isblack(n) (kf_rbtree_color(n) == KF_RBTREE_BLACK)
#define kf_rbtree_isred(n) (kf_rbtree_color(n) == KF_RBTREE_RED)

typedef struct _kf_rbtree_node_t {
	struct _kf_rbtree_node_t *l, *r, *p_color;
} kf_rbtree_node_t;

typedef int (*kf_rbtree_nodecmp_t)(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y);
typedef int (*kf_rbtree_keycmp_t)(const kf_rbtree_node_t *x, const void *k);
typedef void (*kf_rbtree_nodefree_t)(kf_rbtree_node_t *p);

typedef struct _kf_rbtree_t {
	kf_rbtree_node_t *root;
	kf_rbtree_nodecmp_t node_cmp;
	kf_rbtree_keycmp_t key_cmp;
	kf_rbtree_nodefree_t node_free;
} kf_rbtree_t;

kf_rbtree_node_t *kf_rbtree_minnode(kf_rbtree_node_t *node);
kf_rbtree_node_t *kf_rbtree_maxnode(kf_rbtree_node_t *node);

km_result_t kf_rbtree_insert(kf_rbtree_t *tree, kf_rbtree_node_t *node);
void kf_rbtree_remove(kf_rbtree_t *tree, kf_rbtree_node_t *node);
kf_rbtree_node_t *kf_rbtree_find(kf_rbtree_t *tree, const void *key);
void kf_rbtree_free(kf_rbtree_t *tree);

#define kf_rbtree_clear(tree) kf_rbtree_free(tree)

void kf_rbtree_init(kf_rbtree_t *dest,
	kf_rbtree_nodecmp_t node_cmp,
	kf_rbtree_keycmp_t key_cmp,
	kf_rbtree_nodefree_t node_free);

#define kf_rbtree_begin(tree) ((tree)->root ? kf_rbtree_minnode((tree)->root) : NULL)
#define kf_rbtree_end(tree) ((tree)->root ? kf_rbtree_maxnode((tree)->root) : NULL)
kf_rbtree_node_t* kf_rbtree_next(kf_rbtree_node_t* node);

#define kf_rbtree_foreach(i, tree) for (kf_rbtree_node_t *i = kf_rbtree_begin(tree); i; i = kf_rbtree_next(i))

#define kf_rbtree_isempty(tree) ((tree)->root == NULL)

#endif
