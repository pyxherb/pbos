#ifndef _PBOS_KFXX_RBTREE_HH_
#define _PBOS_KFXX_RBTREE_HH_

#include <pbos/kd/assert.h>
#include <pbos/km/panic.h>
#include "fallible_cmp.hh"

namespace kfxx {
	enum class RBColor : bool {
		Black = 0,
		Red = 1
	};

	class _RBTreeBase {
	public:
		struct NodeBase {
			NodeBase *p = nullptr, *l = nullptr, *r = nullptr;
			RBColor color = RBColor::Black;
		};

		NodeBase *_root = nullptr;
		NodeBase *_cached_min_node = nullptr, *_cached_max_node = nullptr;
		size_t _size = 0;

		PBOS_KERNEL_PUBLIC static NodeBase *_get_min_node(NodeBase *node) noexcept;
		PBOS_KERNEL_PUBLIC static NodeBase *_get_max_node(NodeBase *node) noexcept;

		PBOS_FORCEINLINE static bool _is_red(NodeBase *node) noexcept { return node && node->color == RBColor::Red; }
		PBOS_FORCEINLINE static bool _is_black(NodeBase *node) noexcept { return (!node) || node->color == RBColor::Black; }

		PBOS_KERNEL_PUBLIC void _lrot(NodeBase *x) noexcept;
		PBOS_KERNEL_PUBLIC void _rrot(NodeBase *x) noexcept;

		PBOS_KERNEL_PUBLIC void _insert_fixup(NodeBase *node) noexcept;

		PBOS_KERNEL_PUBLIC NodeBase *_remove_fixup(NodeBase *node) noexcept;

		PBOS_KERNEL_PUBLIC static NodeBase *_get_next(const NodeBase *node, const NodeBase *last_node) noexcept;
		PBOS_KERNEL_PUBLIC static NodeBase *_get_prev(const NodeBase *node, const NodeBase *first_node) noexcept;

		PBOS_KERNEL_PUBLIC _RBTreeBase() noexcept;
		PBOS_KERNEL_PUBLIC ~_RBTreeBase();
	};

	template <typename T,
		typename Comparator,
		bool Fallible,
		bool IsThreeway>
	class RBTreeImpl final : protected _RBTreeBase {
	public:
		static_assert(std::is_move_assignable_v<T> || std::is_move_constructible_v<T>, "The key must be move-assignable or move-constructible");
		static_assert(std::is_invocable_v<Comparator, T &, T &>, "The type is not comparable with the comparator");
		struct Node : public _RBTreeBase::NodeBase {
			T rb_value;

			Node() = default;
			PBOS_FORCEINLINE Node(T &&rb_value) : rb_value(std::move(rb_value)) {}
		};

	private:
		using NodeQueryResultType = typename std::conditional<Fallible, Option<Node *>, Node *>::type;

		using ThisType = RBTreeImpl<T, Comparator, Fallible, IsThreeway>;

		Comparator _comparator;

		template <typename U>
		PBOS_FORCEINLINE NodeQueryResultType _get(const U &key) const {
			Node *i = (Node *)_root;

			if constexpr (Fallible) {
				while (i) {
					if constexpr (IsThreeway) {
						auto &&result = _comparator(i->rb_value, key);

						if (result) {
							if (result.value() < 0)
								i = (Node *)i->r;
							else if (result.value() > 0)
								i = (Node *)i->l;
							else
								return i;
						} else
							return NULL_OPTION;
					} else {
						Option<bool> result;

						if ((result = _comparator(i->rb_value, key)).has_value()) {
							if (result.value()) {
#ifndef NDEBUG
								if ((result = _comparator(key, i->rb_value)).has_value()) {
									kd_assert(!result.value());
									i = (Node *)i->r;
								} else {
									return NULL_OPTION;
								}
#else
								i = (Node *)i->r;
#endif
							} else if ((result = _comparator(key, i->rb_value)).has_value()) {
								if (result.value()) {
									i = (Node *)i->l;
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
						if (result < 0)
							i = (Node *)i->r;
						else if (result > 0)
							i = (Node *)i->l;
						else
							return i;
					} else {
						if (_comparator(i->rb_value, key)) {
							kd_assert(!_comparator(key, i->rb_value));
							i = (Node *)i->r;
						} else if (_comparator(key, i->rb_value)) {
							i = (Node *)i->l;
						} else
							return i;
					}
				}
			}
			return nullptr;
		}

		PBOS_FORCEINLINE NodeBase **_get_slot(const T &key, NodeBase *&parent_out) {
			NodeBase **i = (NodeBase **)&_root;
			parent_out = nullptr;

			if constexpr (Fallible) {
				while (*i) {
					parent_out = *i;

					if constexpr (IsThreeway) {
						auto &&result = _comparator(static_cast<Node *>(*i)->rb_value, key);

						if (result) {
							if (result.value() < 0)
								i = (NodeBase **)&static_cast<Node *>(*i)->r;
							else if (result.value() > 0)
								i = (NodeBase **)&static_cast<Node *>(*i)->l;
							else
								return nullptr;
						} else
							return nullptr;
					} else {
						Option<bool> result;

						if ((result = _comparator(static_cast<Node *>(*i)->rb_value, key)).has_value()) {
							if (result.value()) {
#ifndef NDEBUG
								if ((result = _comparator(key, static_cast<Node *>(*i)->rb_value)).has_value()) {
									kd_assert(!result.value());
									i = (NodeBase **)&static_cast<Node *>(*i)->r;
								} else {
									return nullptr;
								}
#else
								i = (NodeBase **)&static_cast<Node *>(*i)->r;
#endif
							} else if ((result = _comparator(key, static_cast<Node *>(*i)->rb_value)).has_value()) {
								if (result.value()) {
									i = (NodeBase **)&static_cast<Node *>(*i)->l;
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

						auto &&result = _comparator(static_cast<Node *>(*i)->rb_value, key);

						if (result < 0) {
							i = (NodeBase **)&((*i)->r);
						} else if (result > 0) {
							i = (NodeBase **)&((*i)->l);
						} else
							return nullptr;
					}
				} else {
					while (*i) {
						parent_out = *i;

						if (_comparator(static_cast<Node *>(*i)->rb_value, key)) {
							kd_assert(!_comparator(key, static_cast<Node *>(*i)->rb_value));
							i = (NodeBase **)&((*i)->r);
						} else if (_comparator(key, static_cast<Node *>(*i)->rb_value)) {
							i = (NodeBase **)&((*i)->l);
						} else
							return nullptr;
					}
				}
			}
			return i;
		}

		// TODO: Use Option for fallible comparison.
		PBOS_FORCEINLINE bool _insert(Node *parent, Node *node) {
			kd_assert(!node->l);
			kd_assert(!node->r);

			if (!_root) {
				_root = node;
				node->color = RBColor::Black;
				goto update_node_caches;
			}

			if constexpr (Fallible) {
				if constexpr (IsThreeway) {
					if (auto &&result = _comparator(node->rb_value, parent->rb_value); result.has_value()) {
						if (result.value() < 0)
							parent->l = node;
						else if (result.value() > 0)
							parent->r = node;
						else
							kd_assert(false);
					} else {
						return false;
					}
				} else {
					Option<bool> result;
					if ((result = _comparator(node->rb_value, parent->rb_value)).has_value()) {
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
					if (result < 0)
						parent->l = node;
					else if (result > 0)
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
			node->color = RBColor::Red;

			_insert_fixup(node);

		update_node_caches:
			_cached_min_node = _get_min_node(_root);
			_cached_max_node = _get_max_node(_root);

			++_size;

			return true;
		}

		PBOS_FORCEINLINE Node *_remove(Node *node) {
			Node *y = (Node *)_remove_fixup(node);

			_cached_min_node = _get_min_node(_root);
			_cached_max_node = _get_max_node(_root);

			--_size;

			return y;
		}

		template <typename U>
		PBOS_FORCEINLINE NodeQueryResultType _find_max_lteq(const U &data) {
			Node *cur_node = (Node *)_root, *max_node = NULL;

			if constexpr (Fallible) {
				while (cur_node) {
					if constexpr (IsThreeway) {
						auto &&result = _comparator(cur_node->rb_value, data);

						if (result.value() < 0) {
							max_node = cur_node;
							cur_node = (Node *)cur_node->r;
						} else if (result.value() > 0)
							cur_node = (Node *)cur_node->l;
						else
							return cur_node;
					} else {
						Option<bool> result;

						if ((result = _comparator(cur_node->rb_value, data)).has_value()) {
							if (result.value()) {
#ifndef NDEBUG
								if ((result = _comparator(data, cur_node->rb_value)).has_value()) {
									kd_assert(!result.value());
									cur_node = (Node *)cur_node->r;
								} else {
									return NULL_OPTION;
								}
#endif
								max_node = cur_node;
								cur_node = (Node *)cur_node->r;
							} else if ((result = _comparator(data, cur_node->rb_value)).has_value()) {
								if (result.value()) {
									cur_node = (Node *)cur_node->l;
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
						if (result < 0) {
							max_node = cur_node;
							cur_node = (Node *)cur_node->r;
						} else if (result > 0)
							cur_node = (Node *)cur_node->l;
						else
							return cur_node;
					} else {
						if (_comparator(cur_node->rb_value, data)) {
							kd_assert(!_comparator(data, cur_node->rb_value));
							max_node = cur_node;
							cur_node = (Node *)cur_node->r;
						} else if (_comparator(data, cur_node->rb_value)) {
							cur_node = (Node *)cur_node->l;
						} else
							return cur_node;
					}
				}
			}

			return max_node;
		}

	public:
		PBOS_FORCEINLINE RBTreeImpl(Comparator &&comparator = {}) : _comparator(std::move(comparator)) {}

		PBOS_FORCEINLINE RBTreeImpl(ThisType &&other)
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

		PBOS_FORCEINLINE ~RBTreeImpl() {
			if (_root)
				km_panic("Destructing RB-tree without all nodes were removed!");
		}

		PBOS_FORCEINLINE ThisType &operator=(ThisType &&other) noexcept {
			if (_root)
				km_panic("Moving RB-tree without original tree's all nodes were removed!");

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

		PBOS_FORCEINLINE NodeQueryResultType find_max_lteq(const T &data) {
			return _find_max_lteq<T>(data);
		}

		template <typename U>
		PBOS_FORCEINLINE NodeQueryResultType find_max_lteq_alt(const U &data) {
			return _find_max_lteq<U>(data);
		}

		PBOS_FORCEINLINE NodeQueryResultType find(const T &key) const {
			return _get<T>(key);
		}

		template <typename U>
		PBOS_FORCEINLINE NodeQueryResultType find_alt(const U &key) const {
			return _get<U>(key);
		}

		/// @brief Insert a node into the tree.
		/// @param node Node to be inserted.
		/// @return Whether the node is inserted successfully, false if node with the same key presents.
		[[nodiscard]] PBOS_FORCEINLINE bool insert(Node *node) {
			// TODO: Use Option for fallible comparison.
			NodeBase *parent = nullptr;
			NodeBase **slot = _get_slot(node->rb_value, parent);

			if (!slot)
				return false;

			return _insert(static_cast<Node *>(parent), node);
		}

		PBOS_FORCEINLINE void insert_unwrap(Node *node) {
			if (!insert(node))
				km_panic("Calling insert_unwrap with insertion failed");
		}

		PBOS_FORCEINLINE void remove(Node *node) {
			Node *y = _remove(node);
			y->l = nullptr;
			y->r = nullptr;
			y->p = nullptr;
		}

		typedef void (*node_deleter_t)(Node *node);

		template <typename D>
		PBOS_FORCEINLINE void clear(D &&deleter) {
			auto d = std::move(deleter);
			if (_root) {
				Node *node = (Node *)_root;
				Node *max_node = (Node *)_get_max_node(node);
				Node *cur_node = (Node *)_get_min_node(node);
				Node *parent = (Node *)node->p;

				while (cur_node != parent) {
					if (cur_node->r) {
						cur_node = (Node *)_get_min_node(cur_node->r);
					} else {
						Node *node_to_delete = cur_node;

						while (cur_node->p && (cur_node == cur_node->p->r)) {
							node_to_delete = cur_node;
							cur_node = (Node *)cur_node->p;
							d(node_to_delete);
						}

						node_to_delete = cur_node;
						cur_node = (Node *)cur_node->p;
						d(node_to_delete);
					}
				}
				_root = nullptr;
				_size = 0;
			}
			_cached_max_node = nullptr;
			_cached_min_node = nullptr;
		}

		PBOS_FORCEINLINE size_t size() const {
			return _size;
		}

		PBOS_FORCEINLINE Comparator &comparator() {
			return _comparator;
		}

		PBOS_FORCEINLINE const Comparator &comparator() const {
			return _comparator;
		}

		static Node *get_next(const Node *node, const Node *last_node) noexcept {
			return (Node *)_get_next((const NodeBase *)node, (const NodeBase *)last_node);
		}

		static Node *get_prev(const Node *node, const Node *first_node) noexcept {
			return (Node *)_get_prev((const NodeBase *)node, (const NodeBase *)first_node);
		}

		struct Iterator {
			Node *node;
			ThisType *tree;
			IteratorDirection direction;

			PBOS_FORCEINLINE Iterator(
				Node *node,
				ThisType *tree,
				IteratorDirection direction)
				: node(node),
				  tree(tree),
				  direction(direction) {}

			Iterator(const Iterator &it) = default;
			PBOS_FORCEINLINE Iterator(Iterator &&it) {
				node = it.node;
				tree = it.tree;
				direction = it.direction;

				it.direction = IteratorDirection::Invalid;
			}
			PBOS_FORCEINLINE Iterator &operator=(const Iterator &rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible Iterator direction");
				node = rhs.node;
				tree = rhs.tree;
				return *this;
			}
			PBOS_FORCEINLINE Iterator &operator=(Iterator &&rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible Iterator direction");
				construct_at<Iterator>(this, std::move(rhs));
				return *this;
			}

			PBOS_FORCEINLINE bool copy(Iterator &dest) noexcept {
				dest = *this;
				return true;
			}

			PBOS_FORCEINLINE Iterator &operator++() {
				if (!node)
					km_panic("Increasing the end Iterator");

				if (direction == IteratorDirection::Forward) {
					node = ThisType::get_next(node, nullptr);
				} else {
					node = ThisType::get_prev(node, nullptr);
				}

				return *this;
			}

			PBOS_FORCEINLINE Iterator operator++(int) {
				Iterator it = *this;
				++(*this);
				return it;
			}

			PBOS_FORCEINLINE Iterator next() {
				Iterator Iterator = *this;

				return ++Iterator;
			}

			PBOS_FORCEINLINE Iterator &operator--() {
				if (direction == IteratorDirection::Forward) {
					if (node == tree->_cached_min_node)
						km_panic("Dereasing the begin Iterator");

					if (!node)
						node = (Node *)tree->_cached_max_node;
					else
						node = ThisType::get_prev(node, nullptr);
				} else {
					if (node == tree->_cached_max_node)
						km_panic("Dereasing the begin Iterator");

					if (!node)
						node = (Node *)tree->_cached_min_node;
					else
						node = ThisType::get_next(node, nullptr);
				}

				return *this;
			}

			PBOS_FORCEINLINE Iterator operator--(int) {
				Iterator it = *this;
				--(*this);
				return it;
			}

			PBOS_FORCEINLINE Iterator prev() {
				Iterator Iterator = *this;

				return --Iterator;
			}

			PBOS_FORCEINLINE bool operator==(const Node *node) const noexcept {
				return this->node == node;
			}

			PBOS_FORCEINLINE bool operator==(const Iterator &it) const {
				if (tree != it.tree)
					km_panic("Cannot compare iterators from different trees");
				return node == it.node;
			}

			PBOS_FORCEINLINE bool operator==(const Iterator &&rhs) const {
				const Iterator it = rhs;
				return *this == it;
			}

			PBOS_FORCEINLINE bool operator!=(const Node *node) const noexcept {
				return this->node != node;
			}

			PBOS_FORCEINLINE bool operator!=(const Iterator &it) const {
				if (tree != it.tree)
					km_panic("Cannot compare iterators from different trees");
				return node != it.node;
			}

			PBOS_FORCEINLINE bool operator!=(Iterator &&rhs) const {
				Iterator it = rhs;
				return *this != it;
			}

			PBOS_FORCEINLINE T &operator*() {
				if (!node)
					km_panic("Deferencing the end Iterator");
				return node->rb_value;
			}

			PBOS_FORCEINLINE T &operator*() const {
				if (!node)
					km_panic("Deferencing the end Iterator");
				return node->rb_value;
			}

			PBOS_FORCEINLINE T *operator->() {
				if (!node)
					km_panic("Deferencing the end Iterator");
				return &node->rb_value;
			}

			PBOS_FORCEINLINE T *operator->() const {
				if (!node)
					km_panic("Deferencing the end Iterator");
				return &node->rb_value;
			}
		};

		PBOS_FORCEINLINE Iterator begin() {
			return Iterator((Node *)_get_min_node(_root), this, IteratorDirection::Forward);
		}
		PBOS_FORCEINLINE Iterator end() {
			return Iterator(nullptr, this, IteratorDirection::Forward);
		}
		PBOS_FORCEINLINE Iterator begin_reversed() {
			return Iterator((Node *)_cached_max_node, this, IteratorDirection::Reversed);
		}
		PBOS_FORCEINLINE Iterator end_reversed() {
			return Iterator(nullptr, this, IteratorDirection::Reversed);
		}

		struct ConstIterator {
			Iterator _iterator;

			PBOS_FORCEINLINE ConstIterator(
				Iterator &&Iterator)
				: _iterator(Iterator) {}

			ConstIterator(const ConstIterator &it) = default;
			PBOS_FORCEINLINE ConstIterator(ConstIterator &&it) : _iterator(std::move(it._iterator)) {
			}
			PBOS_FORCEINLINE ConstIterator &operator=(const ConstIterator &rhs) noexcept {
				_iterator = rhs._iterator;
				return *this;
			}
			PBOS_FORCEINLINE ConstIterator &operator=(ConstIterator &&rhs) noexcept {
				_iterator = std::move(rhs._iterator);
				return *this;
			}

			PBOS_FORCEINLINE bool copy(ConstIterator &dest) noexcept {
				construct_at<ConstIterator>(&dest, *this);
				return true;
			}

			PBOS_FORCEINLINE ConstIterator &operator++() {
				++_iterator;
				return *this;
			}

			PBOS_FORCEINLINE ConstIterator operator++(int) {
				ConstIterator it = *this;
				++(*this);
				return it;
			}

			PBOS_FORCEINLINE ConstIterator next() {
				ConstIterator Iterator = *this;

				return ++Iterator;
			}

			PBOS_FORCEINLINE ConstIterator &operator--() {
				--_iterator;
				return *this;
			}

			PBOS_FORCEINLINE ConstIterator operator--(int) {
				ConstIterator it = *this;
				--(*this);
				return it;
			}

			PBOS_FORCEINLINE ConstIterator prev() {
				ConstIterator Iterator = *this;

				return --Iterator;
			}

			PBOS_FORCEINLINE bool operator==(const ConstIterator &it) const {
				return _iterator == it._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(const ConstIterator &it) const {
				return _iterator != it._iterator;
			}

			PBOS_FORCEINLINE bool operator==(const Node *node) const {
				return _iterator == node;
			}

			PBOS_FORCEINLINE bool operator!=(ConstIterator &&rhs) const {
				ConstIterator it = rhs;
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

		PBOS_FORCEINLINE ConstIterator begin_const() const noexcept {
			return ConstIterator(const_cast<ThisType *>(this)->begin());
		}
		PBOS_FORCEINLINE ConstIterator end_const() const noexcept {
			return ConstIterator(const_cast<ThisType *>(this)->end());
		}
		PBOS_FORCEINLINE ConstIterator begin_const_reversed() const noexcept {
			return ConstIterator(const_cast<ThisType *>(this)->begin_reversed());
		}
		PBOS_FORCEINLINE ConstIterator end_const_reversed() const noexcept {
			return ConstIterator(const_cast<ThisType *>(this)->end_reversed());
		}

		PBOS_FORCEINLINE void remove(const Iterator &Iterator) {
			kd_assert(("Cannot remove the end Iterator", Iterator.node));
			remove(Iterator.node);
		}
	};

	template <typename T, typename Comparator = Cmp<T>, bool IsThreeway = true>
	using RBTree = RBTreeImpl<T, Comparator, false, IsThreeway>;
	template <typename T, typename Comparator = FallibleCmp<T>, bool IsThreeway = true>
	using FallibleRBTree = RBTreeImpl<T, Comparator, true, IsThreeway>;
}

#endif
