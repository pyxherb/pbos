#ifndef _PBOS_KFXX_RBTREE_HH_
#define _PBOS_KFXX_RBTREE_HH_

#include <pbos/km/assert.h>
#include <pbos/km/panic.h>
#include <functional>
#include "fallible_cmp.hh"

#ifdef __cplusplus
namespace kfxx {
	enum class rbcolor_t : bool {
		BLACK = 0,
		RED = 1
	};

	class _rbtree_base {
	public:
		struct node_base {
			node_base *p = nullptr, *l = nullptr, *r = nullptr;
			rbcolor_t color = rbcolor_t::BLACK;
		};

	protected:
		node_base *_root = nullptr;
		node_base *_cached_min_node = nullptr, *_cached_max_node = nullptr;
		size_t _size = 0;

		PBOS_KFXX_API static node_base *_get_min_node(node_base *node);
		PBOS_KFXX_API static node_base *_get_max_node(node_base *node);

		PBOS_FORCEINLINE static bool _is_red(node_base *node) { return node && node->color == rbcolor_t::RED; }
		PBOS_FORCEINLINE static bool _is_black(node_base *node) { return (!node) || node->color == rbcolor_t::BLACK; }

		PBOS_KFXX_API void _lrot(node_base *x);
		PBOS_KFXX_API void _rrot(node_base *x);

		PBOS_KFXX_API void _insert_fixup(node_base *node);

		PBOS_KFXX_API node_base *_remove_fixup(node_base *node);

		PBOS_KFXX_API static node_base *_get_next(const node_base *node, const node_base *lastNode) noexcept;
		PBOS_KFXX_API static node_base *_get_prev(const node_base *node, const node_base *firstNode) noexcept;

		PBOS_KFXX_API _rbtree_base();
		PBOS_KFXX_API ~_rbtree_base();
	};

	template <typename T,
		typename Comparator,
		bool Fallible,
		bool IsThreeway>
	PBOS_REQUIRES_CONCEPT(std::invocable<Comparator, const T &, const T &> &&std::strict_weak_order<Comparator, T, T>)
	class _rbtree_impl : protected _rbtree_base {
	public:
		struct node_t : public _rbtree_base::node_base {
			T rb_value;
		};

		using comparator_type = Comparator;

	protected:
		using node_query_result_t = typename std::conditional<Fallible, option_t<node_t *>, node_t *>::type;

		using this_t = _rbtree_impl<T, Comparator, Fallible, IsThreeway>;

		Comparator _comparator;

		PBOS_FORCEINLINE node_query_result_t _get(const T &key) const {
			node_t *i = (node_t *)_root;

			if constexpr (Fallible) {
				while (i) {
					if constexpr (IsThreeway) {
						auto &&result = _comparator(i->rb_value, key);

						if (result.value() > 0)
							i = (node_t *)i->r;
						else if (result.value() < 0)
							i = (node_t *)i->l;
						else
							return i;
					} else {
						option_t<bool> result;

						if ((result = _comparator(i->rb_value, key)).hasValue()) {
							if (result.value()) {
	#ifndef NDEBUG
								if ((result = _comparator(key, i->rb_value)).hasValue()) {
									kd_assert(!result.value());
									i = (node_t *)i->r;
								} else {
									return NULL_OPTION;
								}
	#else
								i = (node_t *)i->r;
	#endif
							} else if ((result = _comparator(key, i->rb_value)).hasValue()) {
								if (result.value()) {
									i = (node_t *)i->l;
								} else
									return i;
							} else {
								return NULL_OPTION;
							}
						} else {
							return NULL_OPTION;
						}
					}
				}
			} else {
				while (i) {
					if constexpr (IsThreeway) {
						auto &&result = _comparator(i->rb_value, key);
						if (result > 0)
							i = (node_t *)i->r;
						else if (result < 0)
							i = (node_t *)i->l;
						else
							return i;
					} else {
						if (_comparator(i->rb_value, key)) {
							kd_assert(!_comparator(key, i->rb_value));
							i = (node_t *)i->r;
						} else if (_comparator(key, i->rb_value)) {
							i = (node_t *)i->l;
						} else
							return i;
					}
				}
			}
			return nullptr;
		}

		PBOS_FORCEINLINE node_t **_get_slot(const T &key, node_t *&parent_out) {
			node_t **i = (node_t **)&_root;
			parent_out = nullptr;

			if constexpr (Fallible) {
				while (*i) {
					parent_out = *i;

					if constexpr (IsThreeway) {
						auto &&result = _comparator((*i)->rb_value, key);

						if (result.value() > 0)
							i = (node_t **)&(*i)->r;
						else if (result.value() < 0)
							i = (node_t **)&(*i)->l;
						else
							return i;
					} else {
						option_t<bool> result;

						if ((result = _comparator((*i)->rb_value, key)).hasValue()) {
							if (result.value()) {
	#ifndef NDEBUG
								if ((result = _comparator(key, (*i)->rb_value)).hasValue()) {
									kd_assert(!result.value());
									i = (node_t **)&(*i)->r;
								} else {
									return nullptr;
								}
	#else
								i = (node_t **)&(*i)->r;
	#endif
							} else if ((result = _comparator(key, (*i)->rb_value)).hasValue()) {
								if (result.value()) {
									i = (node_t **)&(*i)->l;
								} else
									return i;
							} else {
								return nullptr;
							}
						} else {
							return nullptr;
						}
					}
				}
			} else {
				if constexpr (IsThreeway) {
					while (*i) {
						parent_out = *i;

						auto &&result = _comparator((*i)->rb_value, key);

						if (result > 0) {
							i = (node_t **)&((*i)->r);
						} else if (result < 0) {
							i = (node_t **)&((*i)->l);
						} else
							return nullptr;
					}
				} else {
					while (*i) {
						parent_out = *i;

						if (_comparator((*i)->rb_value, key)) {
							kd_assert(!_comparator(key, (*i)->rb_value));
							i = (node_t **)&((*i)->r);
						} else if (_comparator(key, (*i)->rb_value)) {
							i = (node_t **)&((*i)->l);
						} else
							return nullptr;
					}
				}
			}
			return i;
		}

		PBOS_FORCEINLINE bool _insert(node_t **slot, node_t *parent, node_t *node) {
			node->l = nullptr;
			node->r = nullptr;
			node->p = nullptr;

			if (!_root) {
				_root = node;
				node->color = rbcolor_t::BLACK;
				goto updateNodeCaches;
			}

			if constexpr (Fallible) {
				if constexpr (IsThreeway) {
					if (auto &&result = _comparator(node->rb_value, parent->rb_value); result.hasValue()) {
						if (result.value() > 0)
							parent->l = node;
						else if (result.value() < 0)
							parent->r = node;
						else
							kd_assert(false);
					} else {
						return false;
					}
				} else {
					option_t<bool> result;
					if ((result = _comparator(node->rb_value, parent->rb_value)).hasValue()) {
						if (result.value())
							parent->l = node;
						else
							parent->r = node;
					} else {
						return false;
					}
				}
			} else {
				if (IsThreeway) {
					auto &&result = _comparator(node->rb_value, parent->rb_value);
					if (result > 0)
						parent->l = node;
					else if (result < 0)
						parent->r = node;
					else
						kd_assert(false);
				} else {
					if (_comparator(node->rb_value, parent->rb_value))
						parent->l = node;
					else
						parent->r = node;
				}
			}
			node->p = parent;
			node->color = rbcolor_t::RED;

			_insert_fixup(node);

		updateNodeCaches:
			_cached_min_node = _get_min_node(_root);
			_cached_max_node = _get_max_node(_root);

			++_size;

			return true;
		}

		PBOS_FORCEINLINE node_t *_remove(node_t *node) {
			node_t *y = (node_t *)_remove_fixup(node);

			_cached_min_node = _get_min_node(_root);
			_cached_max_node = _get_max_node(_root);

			--_size;

			return y;
		}

	public:
		PBOS_FORCEINLINE _rbtree_impl(Comparator &&comparator = {}) : _comparator(std::move(comparator)) {}

		PBOS_FORCEINLINE _rbtree_impl(this_t &&other)
			: _comparator(std::move(other._comparator)) {
			_root = other._root;
			_cached_min_node = other._cached_min_node;
			_cached_max_node = other._cached_max_node;
			_size = other._size;

			other._root = nullptr;
			other._cached_min_node = nullptr;
			other._cached_max_node = nullptr;
			other._size = 0;
		}

		inline ~_rbtree_impl() {
			if (_root)
				km_panic("Destructing a non-empty rbtree_t is not allowed");
		}

		PBOS_FORCEINLINE this_t &operator=(this_t &&other) noexcept {
			clear_without_release();

			_root = other._root;
			_cached_min_node = other._cached_min_node;
			_cached_max_node = other._cached_max_node;
			_size = other._size;
			_comparator = std::move(other._comparator);

			other._root = nullptr;
			other._cached_min_node = nullptr;
			other._cached_max_node = nullptr;
			other._size = 0;

			return *this;
		}

		PBOS_FORCEINLINE node_query_result_t find_max_lteq(const T &data) {
			node_t *cur_node = (node_t *)_root, *max_node = NULL;

			if constexpr (Fallible) {
				while (cur_node) {
					if constexpr (IsThreeway) {
						auto &&result = _comparator(cur_node->rb_value, data);

						if (result.value() > 0) {
							max_node = cur_node;
							cur_node = (node_t *)cur_node->r;
						} else if (result.value() < 0)
							cur_node = (node_t *)cur_node->l;
						else
							return cur_node;
					} else {
						option_t<bool> result;

						if ((result = _comparator(cur_node->rb_value, data)).hasValue()) {
							if (result.value()) {
	#ifndef NDEBUG
								if ((result = _comparator(data, cur_node->rb_value)).hasValue()) {
									kd_assert(!result.value());
									cur_node = (node_t *)cur_node->r;
								} else {
									return NULL_OPTION;
								}
	#else
								max_node = cur_node;
								cur_node = (node_t *)curNode->r;
	#endif
							} else if ((result = _comparator(data, cur_node->rb_value)).hasValue()) {
								if (result.value()) {
									cur_node = (node_t *)cur_node->l;
								} else
									return cur_node;
							} else {
								return NULL_OPTION;
							}
						} else {
							return NULL_OPTION;
						}
					}
				}
			} else {
				while (cur_node) {
					if constexpr (IsThreeway) {
						auto &&result = _comparator(cur_node->rb_value, data);
						if (result > 0) {
							max_node = cur_node;
							cur_node = (node_t *)cur_node->r;
						} else if (result < 0)
							cur_node = (node_t *)cur_node->l;
						else
							return cur_node;
					} else {
						if (_comparator(cur_node->rb_value, data)) {
							kd_assert(!_comparator(data, cur_node->rb_value));
							max_node = cur_node;
							cur_node = (node_t *)cur_node->r;
						} else if (_comparator(data, cur_node->rb_value)) {
							cur_node = (node_t *)cur_node->l;
						} else
							return cur_node;
					}
				}
			}

			return max_node;
		}

		PBOS_FORCEINLINE node_query_result_t find(const T &key) const {
			return _get(key);
		}

		/// @brief Insert a node into the tree.
		/// @param node node_t to be inserted.
		/// @return Whether the node is inserted successfully, false if node with the same key presents.
		PBOS_FORCEINLINE void insert(node_t *node) {
			node_t *parent = nullptr, **slot = _get_slot(node->rb_value, parent);

			if (!slot)
				km_panic("Duplicated node insertion in rbtree_t is not allowed");

			_insert(slot, parent, node);
		}

		PBOS_FORCEINLINE void remove(node_t *node) {
			node_t *y = _remove(node);

			y->l = nullptr;
			y->r = nullptr;
			y->p = nullptr;
		}

		typedef void (*node_deleter_t)(node_t *node);

		PBOS_FORCEINLINE void clear(node_deleter_t deleter) {
			if (_root) {
				node_t *node = (node_t *)_root;
				node_t *max_node = (node_t *)_get_max_node(node);
				node_t *cur_node = (node_t *)_get_min_node(node);
				node_t *parent = (node_t *)node->p;

				while (cur_node != parent) {
					if (cur_node->r) {
						cur_node = (node_t *)_get_min_node(cur_node->r);
					} else {
						node_t *nodeToDelete = cur_node;

						while (cur_node->p && (cur_node == cur_node->p->r)) {
							nodeToDelete = cur_node;
							cur_node = (node_t *)cur_node->p;
							deleter(nodeToDelete);
						}

						nodeToDelete = cur_node;
						cur_node = (node_t *)cur_node->p;
						deleter(nodeToDelete);
					}
				}
				_root = nullptr;
				_size = 0;
			}
			_cached_max_node = nullptr;
			_cached_min_node = nullptr;
		}

		PBOS_FORCEINLINE void clear_without_release() {
			_root = nullptr;
			_size = 0;
			_cached_max_node = nullptr;
			_cached_min_node = nullptr;
		}

		PBOS_FORCEINLINE size_t size() {
			return _size;
		}

		PBOS_FORCEINLINE Comparator &comparator() {
			return _comparator;
		}

		PBOS_FORCEINLINE const Comparator &comparator() const {
			return _comparator;
		}

		static node_t *get_next(const node_t *node, const node_t *lastNode) noexcept {
			return (node_t *)_get_next((const node_base *)node, (const node_base *)lastNode);
		}

		static node_t *get_prev(const node_t *node, const node_t *firstNode) noexcept {
			return (node_t *)_get_prev((const node_base *)node, (const node_base *)firstNode);
		}

		struct iterator {
			node_t *node;
			this_t *tree;
			iterator_direction direction;

			PBOS_FORCEINLINE iterator(
				node_t *node,
				this_t *tree,
				iterator_direction direction)
				: node(node),
				  tree(tree),
				  direction(direction) {}

			iterator(const iterator &it) = default;
			PBOS_FORCEINLINE iterator(iterator &&it) {
				node = it.node;
				tree = it.tree;
				direction = it.direction;

				it.direction = iterator_direction::invalid;
			}
			PBOS_FORCEINLINE iterator &operator=(const iterator &rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible iterator direction");
				node = rhs.node;
				tree = rhs.tree;
				return *this;
			}
			PBOS_FORCEINLINE iterator &operator=(iterator &&rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible iterator direction");
				construct_at<iterator>(this, std::move(rhs));
				return *this;
			}

			PBOS_FORCEINLINE bool copy(iterator &dest) noexcept {
				dest = *this;
				return true;
			}

			PBOS_FORCEINLINE iterator &operator++() {
				if (!node)
					km_panic("Increasing the end iterator");

				if (direction == iterator_direction::forward) {
					node = this_t::get_next(node, nullptr);
				} else {
					node = this_t::get_prev(node, nullptr);
				}

				return *this;
			}

			PBOS_FORCEINLINE iterator operator++(int) {
				iterator it = *this;
				++(*this);
				return it;
			}

			PBOS_FORCEINLINE iterator &operator--() {
				if (direction == iterator_direction::forward) {
					if (node == tree->_cached_min_node)
						km_panic("Dereasing the begin iterator");

					if (!node)
						node = (node_t*)tree->_cached_max_node;
					else
						node = this_t::get_prev(node, nullptr);
				} else {
					if (node == tree->_cached_max_node)
						km_panic("Dereasing the begin iterator");

					if (!node)
						node = (node_t*)tree->_cached_min_node;
					else
						node = this_t::get_next(node, nullptr);
				}

				return *this;
			}

			PBOS_FORCEINLINE iterator operator--(int) {
				iterator it = *this;
				--(*this);
				return it;
			}

			PBOS_FORCEINLINE bool operator==(const node_t *node) const noexcept {
				return node == node;
			}

			PBOS_FORCEINLINE bool operator==(const iterator &it) const {
				if (tree != it.tree)
					km_panic("Cannot compare iterators from different trees");
				return node == it.node;
			}

			PBOS_FORCEINLINE bool operator==(const iterator &&rhs) const {
				const iterator it = rhs;
				return *this == it;
			}

			PBOS_FORCEINLINE bool operator!=(const node_t *node) const noexcept {
				return node != node;
			}

			PBOS_FORCEINLINE bool operator!=(const iterator &it) const {
				if (tree != it.tree)
					km_panic("Cannot compare iterators from different trees");
				return node != it.node;
			}

			PBOS_FORCEINLINE bool operator!=(iterator &&rhs) const {
				iterator it = rhs;
				return *this != it;
			}

			PBOS_FORCEINLINE T &operator*() {
				if (!node)
					km_panic("Deferencing the end iterator");
				return node->rb_value;
			}

			PBOS_FORCEINLINE T &operator*() const {
				if (!node)
					km_panic("Deferencing the end iterator");
				return node->rb_value;
			}

			PBOS_FORCEINLINE T *operator->() {
				if (!node)
					km_panic("Deferencing the end iterator");
				return &node->rb_value;
			}

			PBOS_FORCEINLINE T *operator->() const {
				if (!node)
					km_panic("Deferencing the end iterator");
				return &node->rb_value;
			}
		};

		PBOS_FORCEINLINE iterator begin() {
			return iterator((node_t *)_get_min_node(_root), this, iterator_direction::forward);
		}
		PBOS_FORCEINLINE iterator end() {
			return iterator(nullptr, this, iterator_direction::forward);
		}
		PBOS_FORCEINLINE iterator beginreversed() {
			return iterator((node_t *)_cached_max_node, this, iterator_direction::reversed);
		}
		PBOS_FORCEINLINE iterator endreversed() {
			return iterator(nullptr, this, iterator_direction::reversed);
		}

		struct const_iterator {
			iterator _iterator;

			PBOS_FORCEINLINE const_iterator(
				iterator &&iterator)
				: _iterator(iterator) {}

			const_iterator(const const_iterator &it) = default;
			PBOS_FORCEINLINE const_iterator(const_iterator &&it) : _iterator(std::move(it._iterator)) {
			}
			PBOS_FORCEINLINE const_iterator &operator=(const const_iterator &rhs) noexcept {
				_iterator = rhs._iterator;
				return *this;
			}
			PBOS_FORCEINLINE const_iterator &operator=(const_iterator &&rhs) noexcept {
				_iterator = std::move(rhs._iterator);
				return *this;
			}

			PBOS_FORCEINLINE bool copy(const_iterator &dest) noexcept {
				construct_at<const_iterator>(&dest, *this);
				return true;
			}

			PBOS_FORCEINLINE const_iterator &operator++() {
				++_iterator;
				return *this;
			}

			PBOS_FORCEINLINE const_iterator operator++(int) {
				const_iterator it = *this;
				++(*this);
				return it;
			}

			PBOS_FORCEINLINE const_iterator &operator--() {
				--_iterator;
				return *this;
			}

			PBOS_FORCEINLINE const_iterator operator--(int) {
				const_iterator it = *this;
				--(*this);
				return it;
			}

			PBOS_FORCEINLINE bool operator==(const const_iterator &it) const {
				return _iterator == it._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(const const_iterator &it) const {
				return _iterator != it._iterator;
			}

			PBOS_FORCEINLINE bool operator==(const node_t *node) const {
				return _iterator == node;
			}

			PBOS_FORCEINLINE bool operator!=(const_iterator &&rhs) const {
				const_iterator it = rhs;
				return *this != it;
			}

			PBOS_FORCEINLINE const T &operator*() {
				return *_iterator;
			}

			PBOS_FORCEINLINE const T &operator*() const {
				return *_iterator;
			}

			PBOS_FORCEINLINE const T *operator->() {
				return &*_iterator;
			}

			PBOS_FORCEINLINE const T *operator->() const {
				return &*_iterator;
			}
		};

		PBOS_FORCEINLINE const_iterator beginConst() const noexcept {
			return const_iterator(const_cast<this_t *>(this)->begin());
		}
		PBOS_FORCEINLINE const_iterator endConst() const noexcept {
			return const_iterator(const_cast<this_t *>(this)->end());
		}
		PBOS_FORCEINLINE const_iterator beginConstreversed() const noexcept {
			return const_iterator(const_cast<this_t *>(this)->beginreversed());
		}
		PBOS_FORCEINLINE const_iterator endConstreversed() const noexcept {
			return const_iterator(const_cast<this_t *>(this)->endreversed());
		}

		PBOS_FORCEINLINE void remove(const iterator &iterator) {
			kd_assert(("Cannot remove the end iterator", iterator.node));
			remove(iterator.node);
		}
	};

	template <typename T, typename Comparator = std::less<T>, bool IsThreeway = false>
	using rbtree_t = _rbtree_impl<T, Comparator, false, IsThreeway>;
	template <typename T, typename Comparator = fallible_less<T>, bool IsThreeway = false>
	using fallible_rbtree_t = _rbtree_impl<T, Comparator, true, IsThreeway>;
}
#endif

#endif
