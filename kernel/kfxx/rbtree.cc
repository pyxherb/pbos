#include <pbos/kfxx/rbtree.hh>

using namespace kfxx;

PBOS_KFXX_API _rbtree_base::node_base* _rbtree_base::_get_min_node(node_base* node) {
	if (!node)
		return nullptr;

	while (node->l)
		node = node->l;
	return node;
}

PBOS_KFXX_API _rbtree_base::node_base* _rbtree_base::_get_max_node(node_base* node) {
	if (!node)
		return nullptr;

	while (node->r)
		node = node->r;
	return node;
}

PBOS_KFXX_API void _rbtree_base::_lrot(node_base* x) {
	node_base* y = x->r;
	kd_assert(y);

	x->r = y->l;
	if (y->l)
		y->l->p = x;

	y->p = x->p;

	if (!x->p)
		_root = y;
	else if (x->p->l == x)
		x->p->l = y;
	else
		x->p->r = y;

	y->l = x;
	x->p = y;
}

PBOS_KFXX_API void _rbtree_base::_rrot(node_base* x) {
	node_base* y = x->l;
	kd_assert(y);

	x->l = y->r;
	if (y->r)
		y->r->p = x;

	y->p = x->p;
	if (!x->p)
		_root = y;
	else if (x->p->l == x)
		x->p->l = y;
	else
		x->p->r = y;

	y->r = x;
	x->p = y;
}

PBOS_KFXX_API void _rbtree_base::_insert_fixup(node_base* node) {
	node_base* p, * gp = node, * u;  // Parent, grandparent and uncle

	while ((p = gp->p) && _is_red(p)) {
		gp = p->p;

		if (p == gp->l) {
			u = gp->r;

			if (_is_red(u)) {
				p->color = rbcolor_t::BLACK;
				u->color = rbcolor_t::BLACK;
				gp->color = rbcolor_t::RED;
				node = gp;
				continue;
			}
			else {
				if (node == p->r) {
					_lrot(p);
					std::swap(node, p);
				}
				_rrot(gp);
				p->color = rbcolor_t::BLACK;
				gp->color = rbcolor_t::RED;
			}
		}
		else {
			u = gp->l;

			if (_is_red(u)) {
				p->color = rbcolor_t::BLACK;
				u->color = rbcolor_t::BLACK;
				gp->color = rbcolor_t::RED;
				node = gp;
				continue;
			}
			else {
				if (node == p->l) {
					_rrot(p);
					std::swap(node, p);
				}
				_lrot(gp);
				p->color = rbcolor_t::BLACK;
				gp->color = rbcolor_t::RED;
			}
		}
	}

	_root->color = rbcolor_t::BLACK;
}

PBOS_KFXX_API _rbtree_base::node_base* _rbtree_base::_remove_fixup(node_base* node) {
	// Adopted from SGI STL's stl_tree, with some minor improvements.
	node_base* y = node, * x, * p;

	if (!y->l)
		// The node has right child only.
		x = y->r;
	else if (!y->r) {
		// The node has left child only.
		x = y->l;
	}
	else {
		// The node has two children.
		y = _get_min_node(y->r);
		x = y->r;
	}

	if (y != node) {
		node->l->p = y;
		y->l = node->l;

		if (y != node->r) {
			p = y->p;
			if (x)
				x->p = y->p;
			y->p->l = x;
			y->r = node->r;
			node->r->p = y;
		}
		else
			p = y;

		if (_root == node)
			_root = y;
		else if (node->p->l == node)
			node->p->l = y;
		else
			node->p->r = y;

		y->p = node->p;
		std::swap(y->color, node->color);
		y = node;
	}
	else {
		p = y->p;
		if (x)
			x->p = y->p;

		if (_root == node)
			_root = x;
		else if (node->p->l == node)
			node->p->l = x;
		else
			node->p->r = x;
	}

	if (_is_black(y)) {
		while (x != _root && _is_black(x)) {
			if (x == p->l) {
				auto w = p->r;

				if (_is_red(w)) {
					w->color = rbcolor_t::BLACK;
					p->color = rbcolor_t::RED;
					_lrot(p);
					w = p->r;
				}

				if (_is_black(w->l) && _is_black(w->r)) {
					w->color = rbcolor_t::RED;
					x = p;
					p = p->p;
				}
				else {
					if (_is_black(w->r)) {
						if (w->l)
							w->l->color = rbcolor_t::BLACK;
						w->color = rbcolor_t::RED;
						_rrot(w);
						w = p->r;
					}
					w->color = p->color;
					p->color = rbcolor_t::BLACK;
					if (w->r)
						w->r->color = rbcolor_t::BLACK;
					_lrot(p);
					break;
				}
			}
			else {
				auto w = p->l;

				if (_is_red(w)) {
					w->color = rbcolor_t::BLACK;
					p->color = rbcolor_t::RED;
					_rrot(p);
					w = p->l;
				}

				if (_is_black(w->r) && _is_black(w->l)) {
					w->color = rbcolor_t::RED;
					x = p;
					p = p->p;
				}
				else {
					if (_is_black(w->l)) {
						if (w->r)
							w->r->color = rbcolor_t::BLACK;
						w->color = rbcolor_t::RED;
						_lrot(w);
						w = p->l;
					}
					w->color = p->color;
					p->color = rbcolor_t::BLACK;
					if (w->l)
						w->l->color = rbcolor_t::BLACK;
					_rrot(p);
					break;
				}
			}
		}
		if (x)
			x->color = rbcolor_t::BLACK;
	}

	return y;
}

PBOS_KFXX_API _rbtree_base::node_base* _rbtree_base::_get_next(const node_base* node, const node_base* lastNode) noexcept {
	kd_assert(node);

	if (node != lastNode) {
		if (node->r) {
			return _get_min_node(node->r);
		}
		else {
			while (node->p && (node == node->p->r))
				node = node->p;
			return node->p;
		}
	}

	return nullptr;
}

PBOS_KFXX_API _rbtree_base::node_base* _rbtree_base::_get_prev(const node_base* node, const node_base* firstNode) noexcept {
	kd_assert(node);

	if (node != firstNode) {
		if (node->l) {
			return _get_max_node(node->l);
		}
		else {
			while (node->p && (node == node->p->l))
				node = node->p;
			return node->p;
		}
	}

	return nullptr;
}

PBOS_KFXX_API _rbtree_base::_rbtree_base() {
}

PBOS_KFXX_API _rbtree_base::~_rbtree_base() {
	kd_assert(!_root);
}
