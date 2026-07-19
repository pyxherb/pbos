#ifndef _PBOS_KFXX_RADIX_MAP_HH_
#define _PBOS_KFXX_RADIX_MAP_HH_

#include "allocator.hh"
#include "bitset.hh"
#include "option.hh"
#include "scope_guard.hh"
#include "rcobj.hh"

namespace kfxx {
	template <typename K, typename V, size_t R = sizeof(K) * 2>
	class radix_map_t final {
	public:
		static_assert(std::is_integral_v<K>, "The key must be integral type");
		using ThisType = radix_map_t<K, V, R>;

		constexpr static uintmax_t HEIGHT_MAX = sizeof(K) * 8;
		using Height = size_t;	// TODO: Use auto-sized integer type.

		constexpr static size_t N = static_cast<size_t>(1) << R;
		constexpr static K MASK = N - 1;

		constexpr static size_t MAX_STEPS = (HEIGHT_MAX + R - 1) / R;

		struct Node {
			Node *p = nullptr;
			Node *children[N] = {};
			option_array_t<V, N> radix_value;
			Height height = 0;
			size_t num_used_children = 0;
			size_t offset = 0;

			PBOS_FORCEINLINE Node() {
			}
		};

	private:
		kfxx::rc_object_ptr<kfxx::allocator_t> _allocator;
		Height _height;
		size_t _num_nodes;
		Node *_root;

		[[nodiscard]] PBOS_FORCEINLINE Node *_alloc_single_node() {
			Node *node = alloc_and_construct<Node>(_allocator.get());
			if (!node)
				return nullptr;

			return node;
		}

		PBOS_FORCEINLINE void _delete_single_node(Node *node) {
			kfxx::destroy_and_release<Node>(_allocator.get(), node);
		}

		[[nodiscard]] inline bool _grow_height(Height height) {
			Node *original_root = _root, *first_node = nullptr;
			Height original_height = _height;

			kfxx::scope_guard restore_guard([this, &first_node, original_root, original_height]() noexcept {
				for (Node *i = first_node; i; i = i->p) {
					_delete_single_node(i);
				}

				if (original_root)
					original_root->p = nullptr;

				_root = original_root;
				_height = original_height;
			});

			while (_height < height) {
				++_height;

				Node *new_node = _alloc_single_node();
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
			Node *node = _root;
			Height height = _height;

			size_t i = 0;
			kfxx::bitset_t<MAX_STEPS> insertion_flags;

			kfxx::scope_guard sg([this, &insertion_flags, &i, &node]() noexcept {
				while (i && node) {
					--i;
					if (insertion_flags.get_bit(i)) {
						Node *parent = node->p;
						K offset = node->offset;
						--_num_nodes;
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
				Height shift = height - R;
				K offset = (index >> shift) & MASK;

				if ((!node->children[offset]) && (!node->radix_value.has_value(offset))) {
					Node *new_node = _alloc_single_node();
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

			Node *leaf = node;
			K offset = index & MASK;
			kd_assert(!(leaf->children[offset] || leaf->radix_value.has_value(offset)));
			leaf->radix_value.set_value(offset, std::move(data));
			return true;
		}

		[[nodiscard]] inline bool _grow_height_then_grow_node(K index, Height height, V &&data) {
			Node *original_root = _root, *first_node = nullptr;
			Height original_height = _height;

			kfxx::scope_guard restore_guard([this, &first_node, original_root, original_height]() noexcept {
				for (Node *i = first_node; i; i = i->p) {
					_delete_single_node(i);
				}

				if (original_root)
					original_root->p = nullptr;

				_root = original_root;
				_height = original_height;
			});

			while (_height < height) {
				_height += R;

				Node *new_node = _alloc_single_node();
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

		static PBOS_FORCEINLINE Height _required_height(K index) {
			if (index == 0)
				return R;
			Height bits = (Height)(sizeof(K) * 8 - count_leading_zero(index));
			return ((bits + R - 1) / R) * R;
		}

		[[nodiscard]] inline bool _insert(K index, V &&data) {
			Height required_height = _required_height(index);

			if (!_root) {
				Node *node = _alloc_single_node();
				if (!node)
					return false;
				kfxx::scope_guard delete_node_guard([this, node]() noexcept {
					_delete_single_node(node);
				});
				node->height = required_height;
				_height = required_height;

				Height original_height = _height;
				Node *original_root = _root;

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

		[[nodiscard]] inline V &_lookup(K index) const {
			kd_assert(_root);
			Node *node = _root;
			Height height = _height;
			while (height > R) {
				Height shift = height - R;
				K offset = (index >> shift) & MASK;
				if (!node->children[offset]) {
					kd_assert(node->radix_value.has_value(offset));
					return node->radix_value.value(offset);
				}
				node = node->children[offset];
				height -= R;
			}
			K offset = index & MASK;
			kd_assert(node->radix_value.has_value(offset));
			return node->radix_value.value(offset);
		}

		[[nodiscard]] static inline Node *_get_min_node(Node *node) {
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

		[[nodiscard]] static inline Node *_get_max_node(Node *node) {
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

		[[nodiscard]] inline Node *_lookup_upper_bound(K index) const {
			if (!_root)
				return nullptr;

			Node *node = _root;
			Height height = _height;
			while (node && height > R) {
				Height shift = height - R;
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
		PBOS_FORCEINLINE radix_map_t(kfxx::allocator_t *allocator) : _allocator(allocator), _height(0), _num_nodes(0), _root(nullptr) {}
		PBOS_FORCEINLINE ~radix_map_t() {
			if (!_root)
				return;
			Node *cur_node = _get_min_node(_root);
			Node *parent = _root->p;
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
					Node *node_to_delete = cur_node;
					while (cur_node->p &&
						   (cur_node == cur_node->p->children[N - 1] ||
							   [&]() {
								   Node *p = cur_node->p;
								   for (size_t i = cur_node->offset + 1; i < N; ++i) {
									   if (p->children[i]) return false;
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
		}

		[[nodiscard]] PBOS_FORCEINLINE bool insert(K key, V &&value) {
			return _insert(key, std::move(value));
		}

		kfxx::option_t<V> remove(const K &index) noexcept {
			Node *node = _root;
			Height height = _height;

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
				Node *parent = node->p;
				parent->children[node->offset] = nullptr;
				--parent->num_used_children;
				_delete_single_node(node);
				node = parent;
			}

			if (node == _root && node->num_used_children == 0) {
				_delete_single_node(_root);
				_root = nullptr;
				_height = 0;
			}

			return std::move(removed);
		}

		PBOS_FORCEINLINE const V &at(K key) const {
			return _lookup(key);
		}

		PBOS_FORCEINLINE V &at(K key) {
			return _lookup(key);
		}

		PBOS_FORCEINLINE size_t size() {
			return _num_nodes;
		}

		[[nodiscard]] static Node *get_next_node(const Node *node) noexcept {
			const Node *cur = node;
			while (cur->p) {
				K off = cur->offset;
				const Node *parent = cur->p;
				for (K i = off + 1; i < N; ++i) {
					if (parent->children[i]) {
						return _get_min_node(parent->children[i]);
					}
				}
				cur = parent;
			}
			return nullptr;
		}

		[[nodiscard]] static Node *get_prev_node(const Node *node) noexcept {
			const Node *cur = node;
			while (cur->p) {
				K off = cur->offset;
				const Node *parent = cur->p;
				for (K i = off; i-- > 0;) {
					if (parent->children[i]) {
						return _get_max_node(parent->children[i]);
					}
				}
				cur = parent;
			}
			return nullptr;
		}

		struct Iterator {
			Node *node;
			ThisType *tree;
			iterator_direction direction;
			size_t slot_index;

			PBOS_FORCEINLINE Iterator(
				Node *node,
				ThisType *tree,
				iterator_direction direction,
				size_t slot_index)
				: node(node), tree(tree), direction(direction), slot_index(slot_index) {}

			Iterator(const Iterator &it) = default;
			PBOS_FORCEINLINE Iterator(Iterator &&it) {
				node = it.node;
				tree = it.tree;
				direction = it.direction;
				slot_index = it.slot_index;
				it.direction = iterator_direction::Invalid;
			}
			PBOS_FORCEINLINE Iterator &operator=(const Iterator &rhs) noexcept {
				kd_dbgcheck(direction == rhs.direction, "Incompatible iterator direction");
				node = rhs.node;
				tree = rhs.tree;
				slot_index = rhs.slot_index;
				return *this;
			}
			PBOS_FORCEINLINE Iterator &operator=(Iterator &&rhs) noexcept {
				kd_dbgcheck(direction == rhs.direction, "Incompatible iterator direction");
				kfxx::construct_at<Iterator>(this, std::move(rhs));
				return *this;
			}

			PBOS_FORCEINLINE bool copy(Iterator &dest) noexcept {
				dest = *this;
				return true;
			}

			PBOS_FORCEINLINE Iterator &operator++() {
				kd_dbgcheck(node, "Increasing the end iterator");

				if (direction == iterator_direction::Forward) {
					for (size_t i = slot_index + 1; i < N; ++i) {
						if (node->radix_value.has_value(i)) {
							slot_index = i;
							return *this;
						}
					}

					node = ThisType::get_next_node(node);
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
					node = ThisType::get_prev_node(node);
					if (!node) {
						slot_index = N - 1;
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

			PBOS_FORCEINLINE Iterator operator++(int) {
				Iterator it = *this;
				++(*this);
				return it;
			}

			PBOS_FORCEINLINE Iterator next() {
				Iterator iterator = *this;
				return ++iterator;
			}

			PBOS_FORCEINLINE Iterator &operator--() {
				if (direction == iterator_direction::Forward) {
					if (node == tree->_get_min_node(tree->_root) && slot_index == [&]() {
							for (size_t i = 0; i < N; ++i)
								if (node->radix_value.has_value(i)) return i;
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
						node = (Node *)tree->_get_max_node(tree->_root);
					else
						node = ThisType::get_prev_node(node);
					if (!node)
						km_panic("Decreasing invalid");
					for (size_t i = N; i-- > 0;) {
						if (node->radix_value.has_value(i)) {
							slot_index = i;
							return *this;
						}
					}
					km_panic("Corrupted tree");
				} else {
					if (node == tree->_get_max_node(tree->_root) && slot_index == [&]() {
							for (size_t i = N; i-- > 0;)
								if (node->radix_value.has_value(i)) return i;
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
						node = (Node *)tree->_get_min_node(tree->_root);
					else
						node = ThisType::get_next_node(node);
					if (!node)
						km_panic("Decreasing invalid");
					for (size_t i = 0; i < N; ++i) {
						if (node->radix_value.has_value(i)) {
							slot_index = i;
							return *this;
						}
					}
					km_panic("Corrupted tree");
				}
			}

			PBOS_FORCEINLINE Iterator operator--(int) {
				Iterator it = *this;
				--(*this);
				return it;
			}

			PBOS_FORCEINLINE Iterator prev() {
				Iterator iterator = *this;
				return --iterator;
			}

			PBOS_FORCEINLINE bool operator==(const Iterator &it) const {
				if (tree != it.tree)
					km_panic("Cannot compare iterators from different trees");
				return node == it.node && slot_index == it.slot_index;
			}

			PBOS_FORCEINLINE bool operator!=(const Iterator &it) const {
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

		PBOS_FORCEINLINE Iterator begin() {
			Node *node = _get_min_node(_root);
			if (!node) return end();
			size_t slot = 0;
			for (; slot < N; ++slot)
				if (node->radix_value.has_value(slot)) break;
			return Iterator(node, this, iterator_direction::Forward, slot);
		}
		PBOS_FORCEINLINE Iterator end() {
			return Iterator(nullptr, this, iterator_direction::Forward, 0);
		}
		PBOS_FORCEINLINE Iterator begin_reversed() {
			Node *node = _get_max_node(_root);
			if (!node) return end_reversed();
			size_t slot = N - 1;
			for (; slot < N; --slot);
			slot = N - 1;
			while (slot > 0 && !node->radix_value.has_value(slot)) --slot;
			return Iterator(node, this, iterator_direction::Reversed, node->radix_value.has_value(slot) ? slot : 0);
		}
		PBOS_FORCEINLINE Iterator end_reversed() {
			return Iterator(nullptr, this, iterator_direction::Reversed, N - 1);
		}

		struct ConstIterator {
			Iterator _iterator;

			PBOS_FORCEINLINE ConstIterator(Iterator &&iterator) : _iterator(iterator) {}
			ConstIterator(const ConstIterator &it) = default;
			PBOS_FORCEINLINE ConstIterator(ConstIterator &&it) : _iterator(std::move(it._iterator)) {}
			PBOS_FORCEINLINE ConstIterator &operator=(const ConstIterator &rhs) noexcept {
				_iterator = rhs._iterator;
				return *this;
			}
			PBOS_FORCEINLINE ConstIterator &operator=(ConstIterator &&rhs) noexcept {
				_iterator = std::move(rhs._iterator);
				return *this;
			}

			PBOS_FORCEINLINE bool copy(ConstIterator &dest) noexcept {
				kfxx::construct_at<ConstIterator>(&dest, *this);
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
				ConstIterator it = *this;
				return ++it;
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
				ConstIterator it = *this;
				return --it;
			}

			PBOS_FORCEINLINE bool operator==(const ConstIterator &it) const { return _iterator == it._iterator; }
			PBOS_FORCEINLINE bool operator!=(const ConstIterator &it) const { return _iterator != it._iterator; }

			PBOS_FORCEINLINE const V &operator*() { return *_iterator; }
			PBOS_FORCEINLINE const V &operator*() const { return *_iterator; }
			PBOS_FORCEINLINE const V *operator->() { return &*_iterator; }
			PBOS_FORCEINLINE const V *operator->() const { return &*_iterator; }
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
	};
}

#endif
