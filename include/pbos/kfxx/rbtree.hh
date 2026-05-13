#ifndef _PBOS_KFXX_RBTREE_HH_
#define _PBOS_KFXX_RBTREE_HH_

#include <pbos/kd/assert.h>
#include <pbos/km/panic.h>
#include "fallible_cmp.hh"

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

		node_base *_root = nullptr;
		node_base *_cached_min_node = nullptr, *_cached_max_node = nullptr;
		size_t _size = 0;

		PBOS_API static node_base *_get_min_node(node_base *node) noexcept;
		PBOS_API static node_base *_get_max_node(node_base *node) noexcept;

		PBOS_FORCEINLINE static bool _is_red(node_base *node) noexcept { return node && node->color == rbcolor_t::RED; }
		PBOS_FORCEINLINE static bool _is_black(node_base *node) noexcept { return (!node) || node->color == rbcolor_t::BLACK; }

		PBOS_API void _lrot(node_base *x) noexcept;
		PBOS_API void _rrot(node_base *x) noexcept;

		PBOS_API void _insert_fixup(node_base *node) noexcept;

		PBOS_API node_base *_remove_fixup(node_base *node) noexcept;

		PBOS_API static node_base *_get_next(const node_base *node, const node_base *lastnode_t) noexcept;
		PBOS_API static node_base *_get_prev(const node_base *node, const node_base *firstnode_t) noexcept;

		PBOS_API _rbtree_base() noexcept;
		PBOS_API ~_rbtree_base();
	};

	template <typename T,
		typename Comparator,
		bool Fallible,
		bool IsThreeway>
	class _rbtree_impl final : protected _rbtree_base {
	public:
		static_assert(std::is_move_assignable_v<T> || std::is_move_constructible_v<T>, "The key must be move-assignable or move-constructible");
		static_assert(std::is_invocable_v<Comparator, T &, T &>, "The type is not comparable with the comparator");
		struct node_t : public _rbtree_base::node_base {
			T rb_value;

			node_t() = default;
			PBOS_FORCEINLINE node_t(T &&rb_value) : rb_value(std::move(rb_value)) {}
		};

		using comparator_type = Comparator;

	private:
		using node_query_result_t = typename std::conditional<Fallible, option_t<node_t *>, node_t *>::type;

		using this_t = _rbtree_impl<T, Comparator, Fallible, IsThreeway>;

		Comparator _comparator;

		template <typename U>
		PBOS_FORCEINLINE node_query_result_t _get(const U &key) const {
			node_t *i = (node_t *)_root;

			if constexpr (Fallible) {
				while (i) {
					if constexpr (IsThreeway) {
						auto &&result = _comparator(i->rb_value, key);

						if (result.value() < 0)
							i = (node_t *)i->r;
						else if (result.value() > 0)
							i = (node_t *)i->l;
						else
							return i;
					} else {
						option_t<bool> result;

						if ((result = _comparator(i->rb_value, key)).has_value()) {
							if (result.value()) {
	#ifndef NDEBUG
								if ((result = _comparator(key, i->rb_value)).has_value()) {
									kd_assert(!result.value());
									i = (node_t *)i->r;
								} else {
									return NULL_OPTION;
								}
	#else
								i = (node_t *)i->r;
	#endif
							} else if ((result = _comparator(key, i->rb_value)).has_value()) {
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
						if (result < 0)
							i = (node_t *)i->r;
						else if (result > 0)
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

		PBOS_FORCEINLINE node_base **_get_slot(const T &key, node_base *&parent_out) {
			node_base **i = &_root;
			parent_out = nullptr;

			if constexpr (Fallible) {
				while (*i) {
					parent_out = *i;

					if constexpr (IsThreeway) {
						auto &&result = _comparator(static_cast<node_t *>(*i)->rb_value, key);

						if (result.value() < 0)
							i = &(*i)->r;
						else if (result.value() > 0)
							i = &(*i)->l;
						else
							return i;
					} else {
						option_t<bool> result;

						if ((result = _comparator(static_cast<node_t *>(*i)->rb_value, key)).has_value()) {
							if (result.value()) {
	#ifndef NDEBUG
								if ((result = _comparator(key, static_cast<node_t *>(*i)->rb_value)).has_value()) {
									kd_assert(!result.value());
									i = &(*i)->r;
								} else {
									return nullptr;
								}
	#else
								i = (node_base **)&(*i)->r;
	#endif
							} else if ((result = _comparator(key, static_cast<node_t *>(*i)->rb_value)).has_value()) {
								if (result.value()) {
									i = &(*i)->l;
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

						auto &&result = _comparator(static_cast<node_t *>(*i)->rb_value, key);

						if (result < 0) {
							i = &((*i)->r);
						} else if (result > 0) {
							i = &((*i)->l);
						} else
							return nullptr;
					}
				} else {
					while (*i) {
						parent_out = *i;

						if (_comparator(static_cast<node_t *>(*i)->rb_value, key)) {
							kd_assert(!_comparator(key, static_cast<node_t *>(*i)->rb_value));
							i = &((*i)->r);
						} else if (_comparator(key, static_cast<node_t *>(*i)->rb_value)) {
							i = &((*i)->l);
						} else
							return nullptr;
					}
				}
			}
			return i;
		}

		// TODO: Use option_t for fallible comparison.
		PBOS_FORCEINLINE bool _insert(node_t *parent, node_t *node) {
			kd_assert(!node->l);
			kd_assert(!node->r);

			if (!_root) {
				_root = node;
				node->color = rbcolor_t::BLACK;
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
					option_t<bool> result;
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
			node->color = rbcolor_t::RED;

			_insert_fixup(node);

		update_node_caches:
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

		template <typename U>
		PBOS_FORCEINLINE node_query_result_t _find_max_lteq(const U &data) {
			node_t *cur_node = (node_t *)_root, *max_node = NULL;

			if constexpr (Fallible) {
				while (cur_node) {
					if constexpr (IsThreeway) {
						auto &&result = _comparator(cur_node->rb_value, data);

						if (result.value() < 0) {
							max_node = cur_node;
							cur_node = (node_t *)cur_node->r;
						} else if (result.value() > 0)
							cur_node = (node_t *)cur_node->l;
						else
							return cur_node;
					} else {
						option_t<bool> result;

						if ((result = _comparator(cur_node->rb_value, data)).has_value()) {
							if (result.value()) {
	#ifndef NDEBUG
								if ((result = _comparator(data, cur_node->rb_value)).has_value()) {
									kd_assert(!result.value());
									cur_node = (node_t *)cur_node->r;
								} else {
									return NULL_OPTION;
								}
	#else
								max_node = cur_node;
								cur_node = (node_t *)cur_node->r;
	#endif
							} else if ((result = _comparator(data, cur_node->rb_value)).has_value()) {
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
						if (result < 0) {
							max_node = cur_node;
							cur_node = (node_t *)cur_node->r;
						} else if (result > 0)
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

		PBOS_FORCEINLINE ~_rbtree_impl() {
			if (_root)
				km_panic("Destructing RB-tree without all nodes were removed!");
		}

		PBOS_FORCEINLINE this_t &operator=(this_t &&other) noexcept {
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

		PBOS_FORCEINLINE node_query_result_t find_max_lteq(const T &data) {
			return _find_max_lteq<T>(data);
		}

		template <typename U>
		PBOS_FORCEINLINE node_query_result_t find_max_lteq_alt(const U &data) {
			return _find_max_lteq<U>(data);
		}

		PBOS_FORCEINLINE node_query_result_t find(const T &key) const {
			return _get<T>(key);
		}

		template <typename U>
		PBOS_FORCEINLINE node_query_result_t find_alt(const U &key) const {
			return _get<U>(key);
		}

		/// @brief Insert a node into the tree.
		/// @param node node_t to be inserted.
		/// @return Whether the node is inserted successfully, false if node with the same key presents.
		[[nodiscard]] PBOS_FORCEINLINE bool insert(node_t *node) {
			// TODO: Use option_t for fallible comparison.
			node_base *parent = nullptr;
			node_base **slot = _get_slot(node->rb_value, parent);

			if (!slot)
				return false;

			return _insert(static_cast<node_t *>(parent), node);
		}

		PBOS_FORCEINLINE void insert_unwrap(node_t *node) {
			if (!insert(node))
				km_panic("Calling insert_unwrap with insertion failed");
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
						node_t *node_to_delete = cur_node;

						while (cur_node->p && (cur_node == cur_node->p->r)) {
							node_to_delete = cur_node;
							cur_node = (node_t *)cur_node->p;
							deleter(node_to_delete);
						}

						node_to_delete = cur_node;
						cur_node = (node_t *)cur_node->p;
						deleter(node_to_delete);
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

		static node_t *get_next(const node_t *node, const node_t *last_node) noexcept {
			return (node_t *)_get_next((const node_base *)node, (const node_base *)last_node);
		}

		static node_t *get_prev(const node_t *node, const node_t *first_node) noexcept {
			return (node_t *)_get_prev((const node_base *)node, (const node_base *)first_node);
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

				it.direction = iterator_direction::Invalid;
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

				if (direction == iterator_direction::Forward) {
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

			PBOS_FORCEINLINE iterator next() {
				iterator iterator = *this;

				return ++iterator;
			}

			PBOS_FORCEINLINE iterator &operator--() {
				if (direction == iterator_direction::Forward) {
					if (node == tree->_cached_min_node)
						km_panic("Dereasing the begin iterator");

					if (!node)
						node = (node_t *)tree->_cached_max_node;
					else
						node = this_t::get_prev(node, nullptr);
				} else {
					if (node == tree->_cached_max_node)
						km_panic("Dereasing the begin iterator");

					if (!node)
						node = (node_t *)tree->_cached_min_node;
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

			PBOS_FORCEINLINE iterator prev() {
				iterator iterator = *this;

				return --iterator;
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
			return iterator((node_t *)_get_min_node(_root), this, iterator_direction::Forward);
		}
		PBOS_FORCEINLINE iterator end() {
			return iterator(nullptr, this, iterator_direction::Forward);
		}
		PBOS_FORCEINLINE iterator begin_reversed() {
			return iterator((node_t *)_cached_max_node, this, iterator_direction::Reversed);
		}
		PBOS_FORCEINLINE iterator end_reversed() {
			return iterator(nullptr, this, iterator_direction::Reversed);
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

			PBOS_FORCEINLINE const_iterator next() {
				const_iterator iterator = *this;

				return ++iterator;
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

			PBOS_FORCEINLINE const_iterator prev() {
				const_iterator iterator = *this;

				return --iterator;
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

		PBOS_FORCEINLINE const_iterator begin_const() const noexcept {
			return const_iterator(const_cast<this_t *>(this)->begin());
		}
		PBOS_FORCEINLINE const_iterator end_const() const noexcept {
			return const_iterator(const_cast<this_t *>(this)->end());
		}
		PBOS_FORCEINLINE const_iterator begin_const_reversed() const noexcept {
			return const_iterator(const_cast<this_t *>(this)->begin_reversed());
		}
		PBOS_FORCEINLINE const_iterator end_const_reversed() const noexcept {
			return const_iterator(const_cast<this_t *>(this)->end_reversed());
		}

		PBOS_FORCEINLINE void remove(const iterator &iterator) {
			kd_assert(("Cannot remove the end iterator", iterator.node));
			remove(iterator.node);
		}
	};

	template <typename T, typename Comparator = cmp<T>, bool IsThreeway = true>
	using rbtree_t = _rbtree_impl<T, Comparator, false, IsThreeway>;
	template <typename T, typename Comparator = fallible_cmp<T>, bool IsThreeway = true>
	using fallible_rbtree_t = _rbtree_impl<T, Comparator, true, IsThreeway>;
}

#endif
