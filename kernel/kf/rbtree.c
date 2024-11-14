#include <pbos/kf/rbtree.h>

static void kf_rbtree_lrot(kf_rbtree_t *tree, kf_rbtree_node_t *x);
static void kf_rbtree_rrot(kf_rbtree_t *tree, kf_rbtree_node_t *x);
static void kf_rbtree_insert_fixup(kf_rbtree_t *tree, kf_rbtree_node_t *node);
static kf_rbtree_node_t* kf_rbtree_remove_fixup(kf_rbtree_t *tree, kf_rbtree_node_t *node);
static void kf_rbtree_walk_nodes_for_freeing(kf_rbtree_t *tree, kf_rbtree_node_t *node);

void kf_rbtree_init(kf_rbtree_t *dest,
	kf_rbtree_nodecmp_t node_cmp,
	kf_rbtree_nodefree_t node_free) {
	dest->node_cmp = node_cmp;
	dest->node_free = node_free;
	dest->root = NULL;
}

km_result_t kf_rbtree_insert(kf_rbtree_t *tree, kf_rbtree_node_t *node) {
	assert(!node->l);
	assert(!node->r);

	if (!tree->root) {
		tree->root = node;
		kf_rbtree_setcolor(node, KF_RBTREE_BLACK);
		return KM_RESULT_OK;
	}

	kf_rbtree_node_t *x = tree->root, *y = NULL;
	while (x) {
		y = x;
		if (tree->node_cmp(node, x))
			x = x->l;
		else if (tree->node_cmp(x, node))
			x = x->r;
		else
			return KM_MAKEERROR(KM_RESULT_EXISTED);
	}

	if (tree->node_cmp(node, y))
		y->l = node;
	else
		y->r = node;
	kf_rbtree_setparent(node, y);
	kf_rbtree_setcolor(node, KF_RBTREE_RED);

	kf_rbtree_insert_fixup(tree, node);

	return KM_RESULT_OK;
}

void kf_rbtree_remove(kf_rbtree_t *tree, kf_rbtree_node_t *node) {
	kf_rbtree_node_t *y = kf_rbtree_remove_fixup(tree, node);
	y->r = NULL;
	y->l = NULL;

	tree->node_free(y);
}

kf_rbtree_node_t *kf_rbtree_find(kf_rbtree_t *tree, kf_rbtree_node_t *node) {
	kf_rbtree_node_t *i = tree->root;
	while (i) {
		if (tree->node_cmp(i, node))
			i = i->r;
		else if (tree->node_cmp(node, i))
			i = i->l;
		else
			return i;
	}
	return NULL;
}

kf_rbtree_node_t **kf_rbtree_find_slot(kf_rbtree_t *tree, kf_rbtree_node_t *node, kf_rbtree_node_t **p_out) {
	kf_rbtree_node_t **i = &tree->root;
	*p_out = NULL;

	while (*i) {
		*p_out = *i;

		if (tree->node_cmp(*i, node))
			i = &(*i)->r;
		else if (tree->node_cmp(node, *i))
			i = &(*i)->l;
		else
			return NULL;
	}
	return i;
}

kf_rbtree_node_t *kf_rbtree_find_max_lteq_node(kf_rbtree_t *tree, kf_rbtree_node_t *node) {
	kf_rbtree_node_t *cur_node = tree->root, *max_node = NULL;

	while (cur_node) {
		// cur_node < n
		if (tree->node_cmp(cur_node, node)) {
			max_node = cur_node;
			cur_node = cur_node->r;
		} else if(tree->node_cmp(node, cur_node)) {
			cur_node = cur_node->l;
		} else
			return cur_node;
	}

	return max_node;
}

void kf_rbtree_free(kf_rbtree_t *tree) {
	if (tree->root)
		kf_rbtree_walk_nodes_for_freeing(tree, tree->root);
}

kf_rbtree_node_t *kf_rbtree_minnode(kf_rbtree_node_t *node) {
	while (node->l)
		node = node->l;
	return node;
}

kf_rbtree_node_t *kf_rbtree_maxnode(kf_rbtree_node_t *node) {
	while (node->r)
		node = node->r;
	return node;
}

kf_rbtree_node_t *kf_rbtree_next(kf_rbtree_node_t *node) {
	if (!node)
		return NULL;

	if (node->r) {
		node = kf_rbtree_minnode(node->r);
	} else {
		while (kf_rbtree_parent(node) && (node == kf_rbtree_parent(node)->r))
			node = kf_rbtree_parent(node);
		node = kf_rbtree_parent(node);
	}

	return node;
}

static void kf_rbtree_lrot(kf_rbtree_t *tree, kf_rbtree_node_t *x) {
	kf_rbtree_node_t *y = x->r;
	assert(y);

	x->r = y->l;
	if (y->l)
		kf_rbtree_setparent(y->l, x);

	kf_rbtree_setparent(y, kf_rbtree_parent(x));

	if (!kf_rbtree_parent(x))
		tree->root = y;
	else if (kf_rbtree_parent(x)->l == x)
		kf_rbtree_parent(x)->l = y;
	else
		kf_rbtree_parent(x)->r = y;

	y->l = x;
	kf_rbtree_setparent(x, y);
}

static void kf_rbtree_rrot(kf_rbtree_t *tree, kf_rbtree_node_t *x) {
	kf_rbtree_node_t *y = x->l;
	assert(y);

	x->l = y->r;
	if (y->r)
		kf_rbtree_setparent(y->r, x);

	kf_rbtree_setparent(y, kf_rbtree_parent(x));
	if (!kf_rbtree_parent(x))
		tree->root = y;
	else if (kf_rbtree_parent(x)->l == x)
		kf_rbtree_parent(x)->l = y;
	else
		kf_rbtree_parent(x)->r = y;

	y->r = x;
	kf_rbtree_setparent(x, y);
}

static void kf_rbtree_insert_fixup(kf_rbtree_t *tree, kf_rbtree_node_t *node) {
	kf_rbtree_node_t *p, *gp = node, *u;  // Parent, grandparent and uncle

	while ((p = kf_rbtree_parent(gp)) && kf_rbtree_isred(p)) {
		gp = kf_rbtree_parent(p);

		if (p == gp->l) {
			u = gp->r;

			if (kf_rbtree_isred(u)) {
				kf_rbtree_setcolor(p, KF_RBTREE_BLACK);
				kf_rbtree_setcolor(u, KF_RBTREE_BLACK);
				kf_rbtree_setcolor(gp, KF_RBTREE_RED);
				node = gp;
				continue;
			} else {
				if (node == p->r) {
					kf_rbtree_lrot(tree, p);

					{
						kf_rbtree_node_t *tmp;
						tmp = node;
						node = p;
						p = tmp;
					}
				}
				kf_rbtree_rrot(tree, gp);
				kf_rbtree_setcolor(p, KF_RBTREE_BLACK);
				kf_rbtree_setcolor(gp, KF_RBTREE_RED);
			}
		} else {
			u = gp->l;

			if (kf_rbtree_isred(u)) {
				kf_rbtree_setcolor(p, KF_RBTREE_BLACK);
				kf_rbtree_setcolor(u, KF_RBTREE_BLACK);
				kf_rbtree_setcolor(gp, KF_RBTREE_RED);
				node = gp;
				continue;
			} else {
				if (node == p->l) {
					kf_rbtree_rrot(tree, p);

					{
						kf_rbtree_node_t *tmp;
						tmp = node;
						node = p;
						p = tmp;
					}
				}
				kf_rbtree_lrot(tree, gp);
				kf_rbtree_setcolor(p, KF_RBTREE_BLACK);
				kf_rbtree_setcolor(gp, KF_RBTREE_RED);
			}
		}
	}

	kf_rbtree_setcolor(tree->root, KF_RBTREE_BLACK);
}

static kf_rbtree_node_t* kf_rbtree_remove_fixup(kf_rbtree_t *tree, kf_rbtree_node_t *node) {
	// The algorithm was from SGI STL's stl_tree, with minor improvements.
	kf_rbtree_node_t *y = node, *x, *p;

	if (!y->l)
		// The node has right child only.
		x = y->r;
	else if (!y->r) {
		// The node has left child only.
		x = y->l;
	} else {
		// The node has two children.
		y = kf_rbtree_minnode(y->r);
		x = y->r;
	}

	if (y != node) {
		kf_rbtree_setparent(node->l, y);
		y->l = node->l;

		if (y != node->r) {
			p = kf_rbtree_parent(y);
			if (x)
				kf_rbtree_setparent(x, kf_rbtree_parent(y));
			kf_rbtree_parent(y)->l = x;
			y->r = node->r;
			kf_rbtree_setparent(node->r, y);
		} else
			p = y;

		if (tree->root == node)
			tree->root = y;
		else if (kf_rbtree_parent(node)->l == node)
			kf_rbtree_parent(node)->l = y;
		else
			kf_rbtree_parent(node)->r = y;

		kf_rbtree_setparent(y, kf_rbtree_parent(node));

		{
			bool color = kf_rbtree_color(y);
			kf_rbtree_setcolor(y, kf_rbtree_color(node));
			kf_rbtree_setcolor(node, color);
		}
		y = node;
	} else {
		p = kf_rbtree_parent(y);
		if (x)
			kf_rbtree_setparent(x, kf_rbtree_parent(y));

		if (tree->root == node)
			tree->root = x;
		else if (kf_rbtree_parent(node)->l == node)
			kf_rbtree_parent(node)->l = x;
		else
			kf_rbtree_parent(node)->r = x;
	}

	if (kf_rbtree_isblack(y)) {
		while (x != tree->root && kf_rbtree_isblack(x)) {
			if (x == p->l) {
				kf_rbtree_node_t *w = p->r;

				if (kf_rbtree_isred(w)) {
					kf_rbtree_setcolor(w, KF_RBTREE_BLACK);
					kf_rbtree_setcolor(p, KF_RBTREE_RED);
					kf_rbtree_lrot(tree, p);
					w = p->r;
				}

				if (kf_rbtree_isblack(w->l) && kf_rbtree_isblack(w->r)) {
					kf_rbtree_setcolor(w, KF_RBTREE_RED);
					x = p;
					p = kf_rbtree_parent(p);
				} else {
					if (kf_rbtree_isblack(w->r)) {
						if (w->l)
							kf_rbtree_setcolor(w->l, KF_RBTREE_BLACK);
						kf_rbtree_setcolor(w, KF_RBTREE_RED);
						kf_rbtree_rrot(tree, w);
						w = p->r;
					}
					kf_rbtree_setcolor(w, kf_rbtree_color(p));
					kf_rbtree_setcolor(p, KF_RBTREE_BLACK);
					if (w->r)
						kf_rbtree_setcolor(w->r, KF_RBTREE_BLACK);
					kf_rbtree_lrot(tree, p);
					break;
				}
			} else {
				kf_rbtree_node_t *w = p->l;

				if (kf_rbtree_isred(w)) {
					kf_rbtree_setcolor(w, KF_RBTREE_BLACK);
					kf_rbtree_setcolor(p, KF_RBTREE_RED);
					kf_rbtree_rrot(tree, p);
					w = p->l;
				}

				if (kf_rbtree_isblack(w->r) && kf_rbtree_isblack(w->l)) {
					kf_rbtree_setcolor(w, KF_RBTREE_RED);
					x = p;
					p = kf_rbtree_parent(p);
				} else {
					if (kf_rbtree_isblack(w->l)) {
						if (w->r)
							kf_rbtree_setcolor(w->r, KF_RBTREE_BLACK);
						kf_rbtree_setcolor(w, KF_RBTREE_RED);
						kf_rbtree_lrot(tree, w);
						w = p->l;
					}
					kf_rbtree_setcolor(w, kf_rbtree_color(p));
					kf_rbtree_setcolor(p, KF_RBTREE_BLACK);
					if (w->l)
						kf_rbtree_setcolor(w->l, KF_RBTREE_BLACK);
					kf_rbtree_rrot(tree, p);
					break;
				}
			}
		}
		if (x)
			kf_rbtree_setcolor(x, KF_RBTREE_BLACK);
	}

	return y;
}

static void kf_rbtree_walk_nodes_for_freeing(kf_rbtree_t *tree, kf_rbtree_node_t *node) {
	if (node->l)
		kf_rbtree_walk_nodes_for_freeing(tree, node->l);
	if (node->r)
		kf_rbtree_walk_nodes_for_freeing(tree, node->r);
	tree->node_free(node);
}
