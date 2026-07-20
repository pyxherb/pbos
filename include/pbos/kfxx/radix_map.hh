#ifndef _PBOS_KFXX_RADIX_MAP_HH_
#define _PBOS_KFXX_RADIX_MAP_HH_

#include "allocator.hh"
#include "bitops.hh"
#include "bitset.hh"
#include "option.hh"
#include "rcobj.hh"
#include "scope_guard.hh"

namespace kfxx {
	template <typename K, typename V, size_t R = sizeof(K) * 2>
	class radix_map_t final {
	public:
		static_assert(std::is_integral_v<K>, "The key must be integral type");
		using this_type = radix_map_t<K, V, R>;

		constexpr static uintmax_t HEIGHT_MAX = sizeof(K) * 8;
		// TODO: Use auto sized integer
		using height_type = size_t;

		constexpr static size_t N = static_cast<size_t>(1) << R;
		constexpr static K MASK = N - 1;

		constexpr static size_t MAX_STEPS = (HEIGHT_MAX + R - 1) / R;

		struct node_t {
			node_t *p = nullptr;
			node_t *children[N] = {};
			option_array_t<V, N> radix_value;
			height_type height = 0;
			size_t num_used_children = 0;
			K offset = 0;

			PBOS_FORCEINLINE node_t() {
			}
		};

	private:
		kfxx::rc_object_ptr<kfxx::allocator_t> _allocator;
		height_type _height;
		size_t _size;
		node_t *_root;

		[[nodiscard]] PBOS_FORCEINLINE node_t *_alloc_single_node() {
			node_t *node = alloc_and_construct<node_t>(_allocator.get());
			if (!node)
				return nullptr;

			return node;
		}

		PBOS_FORCEINLINE void _delete_single_node(node_t *node) {
			kfxx::destroy_and_release<node_t>(_allocator.get(), node);
		}

		[[nodiscard]] inline bool _grow_height(height_type height) {
			node_t *original_root = _root, *first_node = nullptr;
			height_type original_height = _height;

			kfxx::scope_guard restore_guard([this, &first_node, original_root, original_height]() noexcept {
				for (node_t *i = first_node; i; i = i->p) {
					_delete_single_node(i);
				}

				if (original_root)
					original_root->p = nullptr;

				_root = original_root;
				_height = original_height;
			});

			while (_height < height) {
				++_height;

				node_t *new_node = _alloc_single_node();
				if (!new_node)
					return false;

				if (!first_node)
					first_node = new_node;

				new_node->height = _height;
				new_node->offset = 0;

				_root->p = new_node;
				new_node->children[0] = _root;
				++new_node->num_used_children;
				_root = new_node;
			}

			restore_guard.release();

			return true;
		}

		[[nodiscard]] inline bool _grow_node(K index, V &&data) {
			node_t *node = _root;
			height_type height = _height;

			size_t i = 0;
			kfxx::bitset_t<MAX_STEPS> insertion_flags;

			kfxx::scope_guard sg([this, &insertion_flags, &i, &node]() noexcept {
				while (i && node) {
					--i;
					if (insertion_flags.get_bit(i)) {
						node_t *parent = node->p;
						K offset = node->offset;
						_delete_single_node(node);
						if (parent) {
							parent->children[offset] = nullptr;
						}
						node = parent;
					} else {
						node = node->p;
					}
				}
			});

			while (height > R) {
				height_type shift = height - R;
				K offset = (index >> shift) & MASK;

				if ((!node->children[offset]) && (!node->radix_value.has_value(offset))) {
					node_t *new_node = _alloc_single_node();
					if (!new_node)
						return false;
					new_node->height = height - R;
					++node->num_used_children;
					new_node->offset = offset;
					new_node->p = node;
					node->children[offset] = new_node;

					node = new_node;
					insertion_flags.set_bit(i);
				} else {
					kd_assert(!node->radix_value.has_value(offset));
					node = node->children[offset];
				}
				++i;
				height -= R;
			}

			sg.release();

			node_t *leaf = node;
			K offset = index & MASK;
			kd_assert(!(leaf->children[offset] || leaf->radix_value.has_value(offset)));
			leaf->radix_value.set_value(offset, std::move(data));
			++leaf->num_used_children;
			++_size;
			return true;
		}

		[[nodiscard]] inline bool _grow_height_then_grow_node(K index, height_type height, V &&data) {
			node_t *original_root = _root, *first_node = nullptr;
			height_type original_height = _height;

			kfxx::scope_guard restore_guard([this, &first_node, original_root, original_height]() noexcept {
				for (node_t *i = first_node; i; i = i->p) {
					_delete_single_node(i);
				}

				if (original_root)
					original_root->p = nullptr;

				_root = original_root;
				_height = original_height;
			});

			while (_height < height) {
				_height += R;

				node_t *new_node = _alloc_single_node();
				if (!new_node)
					return false;

				if (!first_node)
					first_node = new_node;

				new_node->height = _height;
				new_node->offset = 0;

				_root->p = new_node;
				new_node->children[0] = _root;
				++new_node->num_used_children;
				_root = new_node;
			}

			if (!_grow_node(index, std::move(data)))
				return false;

			restore_guard.release();

			return true;
		}

		static PBOS_FORCEINLINE height_type _required_height(K index) {
			if (index == 0)
				return R;
			height_type bits = (height_type)(sizeof(K) * 8 - count_leading_zero(index));
			return ((bits + R - 1) / R) * R;
		}

		[[nodiscard]] inline bool _insert(K index, V &&data) {
			height_type required_height = _required_height(index);

			if (!_root) {
				node_t *node = _alloc_single_node();
				if (!node)
					return false;
				kfxx::scope_guard delete_node_guard([this, node]() noexcept {
					_delete_single_node(node);
				});
				node->height = required_height;
				_height = required_height;

				height_type original_height = _height;
				node_t *original_root = _root;

				kfxx::scope_guard restore_props_guard([this, original_root, original_height]() noexcept {
					_root = original_root;
					_height = original_height;
				});

				_root = node;
				if (!_grow_node(index, std::move(data)))
					return false;
				restore_props_guard.release();
				delete_node_guard.release();
				return true;
			}

			if (required_height > _height)
				return _grow_height_then_grow_node(index, required_height, std::move(data));

			return _grow_node(index, std::move(data));
		}

		[[nodiscard]] inline kfxx::option_t<std::pair<node_t *, size_t>> _lookup(K index) const {
			if (!_root)
				return kfxx::nullopt;
			node_t *node = _root;
			height_type height = _height;
			while (height > R) {
				height_type shift = height - R;
				K offset = (index >> shift) & MASK;
				if (!node->children[offset]) {
					if (!node->radix_value.has_value(offset))
						return kfxx::nullopt;
					return std::pair<node_t *, size_t>(node, offset);
				}
				node = node->children[offset];
				height -= R;
			}
			K offset = index & MASK;
			if (!node->radix_value.has_value(offset))
				return kfxx::nullopt;
			return std::pair<node_t *, size_t>(node, offset);
		}

		[[nodiscard]] static inline node_t *_get_min_node(node_t *node) {
			kd_assert(node);

			while (true) {
				bool has_child = false;
				for (size_t i = 0; i < N; ++i) {
					if (node->children[i]) {
						node = node->children[i];
						has_child = true;
						break;
					}
				}
				if (!has_child)
					break;
			}

			return node;
		}

		[[nodiscard]] static inline node_t *_get_max_node(node_t *node) {
			kd_assert(node);

			while (true) {
				bool has_child = false;
				for (size_t i = N; i-- > 0;) {
					if (node->children[i]) {
						node = node->children[i];
						has_child = true;
						break;
					}
				}
				if (!has_child)
					break;
			}

			return node;
		}

		[[nodiscard]] inline node_t *_lookup_upper_bound(K index) const {
			if (!_root)
				return nullptr;

			node_t *node = _root;
			height_type height = _height;
			while (node && height > R) {
				height_type shift = height - R;
				K offset = (index >> shift) & MASK;
				if (node->children[offset]) {
					node = node->children[offset];
					height -= R;
				} else {
					for (K i = offset + 1; i < N; ++i) {
						if (node->children[i])
							return _get_min_node(node->children[i]);
					}
					break;
				}
			}

			if (node && height == R) {
				K offset = index & MASK;
				for (K i = offset; i < N; ++i) {
					if (node->radix_value.has_value(i))
						return node;
				}

				while (node && node->p) {
					K off = node->offset;
					node = node->p;
					for (K i = off + 1; i < N; ++i) {
						if (node->children[i])
							return _get_min_node(node->children[i]);
					}
				}
			}

			return nullptr;
		}

	public:
		PBOS_FORCEINLINE radix_map_t(kfxx::allocator_t *allocator) : _allocator(allocator), _height(0), _size(0), _root(nullptr) {}
		PBOS_FORCEINLINE ~radix_map_t() {
			clear();
		}

		inline void clear() noexcept {
			if (!_root)
				return;
			node_t *cur_node = _get_min_node(_root);
			node_t *parent = _root->p;
			bool walked_root_node = false;

			while (cur_node != parent) {
				bool has_unvisited = false;
				for (size_t i = 0; i < N; ++i) {
					if (cur_node->children[i]) {
						cur_node = _get_min_node(cur_node->children[i]);
						has_unvisited = true;
						break;
					}
				}
				if (!has_unvisited) {
					node_t *node_to_delete = cur_node;
					while (cur_node->p &&
						   (cur_node == cur_node->p->children[N - 1] ||
							   [&]() {
								   node_t *p = cur_node->p;
								   for (size_t i = cur_node->offset + 1; i < N; ++i) {
									   if (p->children[i])
										   return false;
								   }
								   return true;
							   }())) {
						node_to_delete = cur_node;
						cur_node = cur_node->p;
						_delete_single_node(node_to_delete);
					}
					node_to_delete = cur_node;
					cur_node = cur_node->p;
					_delete_single_node(node_to_delete);
				}
			}

			_root = nullptr;
			_size = 0;
		}

		[[nodiscard]] PBOS_FORCEINLINE bool insert(K key, V &&value) {
			return _insert(key, std::move(value));
		}

		inline option_t<V> remove(const K &index) noexcept {
			node_t *node = _root;
			height_type height = _height;

			while (height > R) {
				K offset = (index >> (height - R)) & MASK;
				if (!node->children[offset])
					return kfxx::nullopt;
				node = node->children[offset];
				height -= R;
			}

			K offset = index & MASK;
			if (!node->radix_value.has_value(offset))
				return kfxx::nullopt;

			V removed = std::move(node->radix_value.value(offset));
			node->radix_value.move(offset);
			--node->num_used_children;

			while (node->num_used_children == 0 && node != _root) {
				node_t *parent = node->p;
				parent->children[node->offset] = nullptr;
				--parent->num_used_children;
				_delete_single_node(node);
				node = parent;
			}

			if (node == _root && (!node->num_used_children)) {
				_delete_single_node(_root);
				_root = nullptr;
				_height = 0;
			}

			--_size;

			return std::move(removed);
		}

		PBOS_FORCEINLINE const V &at(K key) const {
			auto result = _lookup(key);
			kd_assert(result.has_value());
			return result->first->radix_value.value(result->second);
		}

		PBOS_FORCEINLINE V &at(K key) {
			auto result = _lookup(key);
			kd_assert(result.has_value());
			return result->first->radix_value.value(result->second);
		}

		PBOS_FORCEINLINE bool contains(K key) const noexcept {
			return _lookup(key).has_value();
		}

		PBOS_FORCEINLINE size_t size() {
			return _size;
		}

		static node_t *get_next_node(const node_t *leaf) {
			const node_t *cur = leaf;
			while (cur->p) {
				size_t off = cur->offset;
				const node_t *parent = cur->p;
				for (size_t i = off + 1; i < N; ++i) {
					if (parent->children[i])
						return _get_min_node(parent->children[i]);
				}
				cur = parent;
			}
			return nullptr;
		}

		static node_t *get_prev_node(const node_t *leaf) {
			const node_t *cur = leaf;
			while (cur->p) {
				size_t off = cur->offset;
				const node_t *parent = cur->p;
				for (size_t i = off; i-- > 0;) {
					if (parent->children[i])
						return _get_max_node(parent->children[i]);
				}
				cur = parent;
			}
			return nullptr;
		}

		struct iterator {
			node_t *node;
			this_type *tree;
			iterator_direction direction;
			size_t slot_index;

			PBOS_FORCEINLINE iterator(
				node_t *node,
				this_type *tree,
				iterator_direction direction,
				size_t slot_index)
				: node(node), tree(tree), direction(direction), slot_index(slot_index) {}

			iterator(const iterator &it) = default;
			PBOS_FORCEINLINE iterator(iterator &&it) {
				node = it.node;
				tree = it.tree;
				direction = it.direction;
				slot_index = it.slot_index;
				it.direction = iterator_direction::Invalid;
			}
			PBOS_FORCEINLINE iterator &operator=(const iterator &rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible iterator direction");
				node = rhs.node;
				tree = rhs.tree;
				slot_index = rhs.slot_index;
				return *this;
			}
			PBOS_FORCEINLINE iterator &operator=(iterator &&rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible iterator direction");
				kfxx::construct_at<iterator>(this, std::move(rhs));
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
					for (size_t i = slot_index + 1; i < N; ++i) {
						if (node->radix_value.has_value(i)) {
							slot_index = i;
							return *this;
						}
					}

					node = this_type::get_next_node(node);
					if (!node) {
						slot_index = 0;
						return *this;
					}

					for (size_t i = 0; i < N; ++i) {
						if (node->radix_value.has_value(i)) {
							slot_index = i;
							return *this;
						}
					}

					km_panic("Corrupted tree: empty leaf in forward traversal");
				} else {
					for (size_t i = slot_index; i-- > 0;) {
						if (node->radix_value.has_value(i)) {
							slot_index = i;
							return *this;
						}
					}
					node = this_type::get_prev_node(node);
					if (!node) {
						slot_index = 0;
						return *this;
					}
					for (size_t i = N; i-- > 0;) {
						if (node->radix_value.has_value(i)) {
							slot_index = i;
							return *this;
						}
					}
					km_panic("Corrupted tree: empty leaf in backward traversal");
				}
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
					if (node == tree->_get_min_node(tree->_root) && slot_index == [&]() {
							for (size_t i = 0; i < N; ++i) {
								if (node->radix_value.has_value(i))
									return i;
							}
							return size_t(0);
						}())
						km_panic("Decreasing the begin iterator");

					for (size_t i = slot_index; i-- > 0;) {
						if (node->radix_value.has_value(i)) {
							slot_index = i;
							return *this;
						}
					}
					if (!node)
						node = (node_t *)tree->_get_max_node(tree->_root);
					else
						node = this_type::get_prev_node(node);
					if (!node) km_panic("Decreasing invalid");
					for (size_t i = N; i-- > 0;) {
						if (node->radix_value.has_value(i)) {
							slot_index = i;
							return *this;
						}
					}
					km_panic("Corrupted tree");
				} else {
					if (node == tree->_get_max_node(tree->_root) && slot_index == [&]() {
							for (size_t i = N; i-- > 0;) {
								if (node->radix_value.has_value(i))
									return i;
							}
							return size_t(N - 1);
						}())
						km_panic("Decreasing the begin iterator");

					for (size_t i = slot_index + 1; i < N; ++i) {
						if (node->radix_value.has_value(i)) {
							slot_index = i;
							return *this;
						}
					}
					if (!node)
						node = (node_t *)tree->_get_min_node(tree->_root);
					else
						node = this_type::get_next_node(node);
					if (!node) km_panic("Decreasing invalid");
					for (size_t i = 0; i < N; ++i) {
						if (node->radix_value.has_value(i)) {
							slot_index = i;
							return *this;
						}
					}
					km_panic("Corrupted tree");
				}
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

			PBOS_FORCEINLINE bool operator==(const iterator &it) const {
				if (tree != it.tree)
					km_panic("Cannot compare iterators from different trees");
				return node == it.node && slot_index == it.slot_index;
			}

			PBOS_FORCEINLINE bool operator!=(const iterator &it) const {
				return !(*this == it);
			}

			PBOS_FORCEINLINE V &operator*() {
				if (!node)
					km_panic("Dereferencing the end iterator");
				return node->radix_value.value(slot_index);
			}

			PBOS_FORCEINLINE const V &operator*() const {
				if (!node)
					km_panic("Dereferencing the end iterator");
				return node->radix_value.value(slot_index);
			}

			PBOS_FORCEINLINE V *operator->() {
				if (!node)
					km_panic("Dereferencing the end iterator");
				return &node->radix_value.value(slot_index);
			}

			PBOS_FORCEINLINE const V *operator->() const {
				if (!node)
					km_panic("Dereferencing the end iterator");
				return &node->radix_value.value(slot_index);
			}
		};

		PBOS_FORCEINLINE iterator begin() {
			node_t *node = _get_min_node(_root);
			if (!node)
				return end();
			size_t slot = 0;
			for (; slot < N; ++slot) {
				if (node->radix_value.has_value(slot))
					break;
			}
			return iterator(node, this, iterator_direction::Forward, slot);
		}
		PBOS_FORCEINLINE iterator end() {
			return iterator(nullptr, this, iterator_direction::Forward, 0);
		}
		PBOS_FORCEINLINE iterator begin_reversed() {
			node_t *node = _root ? _get_max_node(_root) : nullptr;
			if (!node)
				return end_reversed();
			size_t slot = N - 1;
			while (slot && !node->radix_value.has_value(slot))
				--slot;
			return iterator(node, this, iterator_direction::Reversed, node->radix_value.has_value(slot) ? slot : 0);
		}

		PBOS_FORCEINLINE iterator end_reversed() {
			return iterator(nullptr, this, iterator_direction::Reversed, 0);
		}

		struct const_iterator {
			iterator _iterator;

			PBOS_FORCEINLINE const_iterator(iterator &&iterator) : _iterator(iterator) {}
			const_iterator(const const_iterator &it) = default;
			PBOS_FORCEINLINE const_iterator(const_iterator &&it) : _iterator(std::move(it._iterator)) {}
			PBOS_FORCEINLINE const_iterator &operator=(const const_iterator &rhs) noexcept {
				_iterator = rhs._iterator;
				return *this;
			}
			PBOS_FORCEINLINE const_iterator &operator=(const_iterator &&rhs) noexcept {
				_iterator = std::move(rhs._iterator);
				return *this;
			}

			PBOS_FORCEINLINE bool copy(const_iterator &dest) noexcept {
				kfxx::construct_at<const_iterator>(&dest, *this);
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
				const_iterator it = *this;
				return ++it;
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
				const_iterator it = *this;
				return --it;
			}

			PBOS_FORCEINLINE bool operator==(const const_iterator &it) const {
				return _iterator == it._iterator;
			}
			PBOS_FORCEINLINE bool operator!=(const const_iterator &it) const {
				return _iterator != it._iterator;
			}

			PBOS_FORCEINLINE const V &operator*() {
				return *_iterator;
			}
			PBOS_FORCEINLINE const V &operator*() const {
				return *_iterator;
			}
			PBOS_FORCEINLINE const V *operator->() {
				return &*_iterator;
			}
			PBOS_FORCEINLINE const V *operator->() const {
				return &*_iterator;
			}
		};

		PBOS_FORCEINLINE const_iterator begin_const() const noexcept {
			return const_iterator(const_cast<this_type *>(this)->begin());
		}
		PBOS_FORCEINLINE const_iterator end_const() const noexcept {
			return const_iterator(const_cast<this_type *>(this)->end());
		}
		PBOS_FORCEINLINE const_iterator begin_const_reversed() const noexcept {
			return const_iterator(const_cast<this_type *>(this)->begin_reversed());
		}
		PBOS_FORCEINLINE const_iterator end_const_reversed() const noexcept {
			return const_iterator(const_cast<this_type *>(this)->end_reversed());
		}

		PBOS_FORCEINLINE iterator find(K key) noexcept {
			auto result = _lookup(key);
			if (!result.has_value())
				return end();
			return iterator(result->first, this, iterator_direction::Forward, result->second);
		}

		PBOS_FORCEINLINE const_iterator find(K key) const noexcept {
			return const_iterator(const_cast<this_type>(this)->find(key));
		}
	};
}

#endif
