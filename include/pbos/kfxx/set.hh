#ifndef _PBOS_KFXX_SET_H_
#define _PBOS_KFXX_SET_H_

#include <concepts>
#include "allocator.hh"
#include "option.hh"
#include "rbtree.hh"
#include "scope_guard.hh"
#include "rcobj.hh"

namespace kfxx {
	template <typename T, typename Comparator, bool Fallible, bool IsThreeway>
	class _set_impl final {
	private:
		static_assert(std::is_move_constructible_v<T>, "The element must be move-constructible");
		using tree_t = std::conditional_t<Fallible, fallible_rbtree_t<T, Comparator, IsThreeway>, rbtree_t<T, Comparator, IsThreeway>>;
		tree_t _tree;
		kfxx::rc_object_ptr_t<kfxx::allocator_t> _allocator;
		using this_t = _set_impl<T, Comparator, Fallible, IsThreeway>;

	public:
		using remove_result_t = typename std::conditional_t<Fallible, bool, void>;
		using element_query_result_t = typename std::conditional_t<Fallible, option_t<T &>, T &>;
		using const_element_query_result_t = typename std::conditional_t<Fallible, option_t<const T &>, const T &>;
		using contains_result_t = typename std::conditional_t<Fallible, option_t<bool>, bool>;

		using node_t = typename tree_t::node_t;

		PBOS_FORCEINLINE _set_impl(allocator_t *allocator, Comparator &&comparator = {}) : _tree(std::move(comparator)) {
		}
		PBOS_FORCEINLINE _set_impl(this_t &&rhs) : _tree(std::move(rhs._tree)) {
		}
		PBOS_FORCEINLINE ~_set_impl() {
		}

		PBOS_FORCEINLINE this_t &operator=(this_t &&dest) noexcept {
			_tree = std::move(dest._tree);
			return *this;
		}

		[[nodiscard]] PBOS_FORCEINLINE bool insert(T &&value) {
			typename tree_t::node_t *node = kfxx::alloc_and_construct<node_t>(allocator(), std::move(value));

			if (!_tree.insert(node)) {
				kfxx::destroy_and_release(allocator(), node);
				return false;
			}

			return true;
		}

		PBOS_FORCEINLINE remove_result_t remove(const T &key) {
			return remove_alt<T>(key);
		}

		template <typename U>
		PBOS_FORCEINLINE remove_result_t remove_alt(const U &key) {
			if constexpr (Fallible) {
				auto node = _tree.template find_alt<U>(key);

				if (!node.has_value())
					return false;

				if (auto v = node.value(); v)
					_tree.remove(node.value());

				return true;
			} else {
				auto node = _tree.template find_alt<U>(key);

				if (node)
					_tree.remove(node);
			}
		}

		PBOS_FORCEINLINE void verify() {
			_tree.verify();
		}

		PBOS_FORCEINLINE size_t size() const {
			return _tree.size();
		}

		PBOS_FORCEINLINE allocator_t *allocator() const {
			return _allocator.get();
		}

		PBOS_FORCEINLINE void replace_allocator(allocator_t *rhs) noexcept {
			_tree.replace_allocator(rhs);
		}

		PBOS_FORCEINLINE Comparator &comparator() {
			return _tree.comparator();
		}

		PBOS_FORCEINLINE const Comparator &comparator() const {
			return _tree.comparator();
		}

		PBOS_FORCEINLINE void clear() {
			_tree.clear();
		}

		PBOS_FORCEINLINE element_query_result_t at(const T &key) {
			return at_alt<T>(key);
		}

		template <typename U>
		PBOS_FORCEINLINE element_query_result_t at_alt(const U &key) {
			if constexpr (Fallible) {
				auto node = _tree.template find_alt<U>(key);

				if (!node.has_value())
					return NULL_OPTION;

				assert(node.value());

				return node.value()->value;
			} else {
				auto node = _tree.template find_alt<U>(key);

				assert(node);

				return node->rb_value;
			}
		}

		PBOS_FORCEINLINE const_element_query_result_t at(const T &key) const {
			return at_alt<T>(key);
		}

		template <typename U>
		PBOS_FORCEINLINE const_element_query_result_t at_alt(const U &key) const {
			if constexpr (Fallible) {
				auto node = _tree.template find_alt<U>(key);

				if (!node.has_value())
					return NULL_OPTION;

				assert(node.value());

				return node.value()->value;
			} else {
				auto node = _tree.template find_alt<U>(key);

				assert(node);

				return node->rb_value;
			}
		}

		struct iterator {
			typename tree_t::iterator _iterator;
			PBOS_FORCEINLINE iterator(typename tree_t::iterator &&iterator_in) : _iterator(iterator_in) {
			}
			iterator(const iterator &rhs) = default;
			iterator(iterator &&rhs) = default;
			iterator &operator=(const iterator &rhs) = default;
			iterator &operator=(iterator &&rhs) = default;

			PBOS_FORCEINLINE bool operator==(const iterator &rhs) const {
				return _iterator == rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator==(iterator &&rhs) const {
				return _iterator == rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(const iterator &rhs) const {
				return _iterator != rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(iterator &&rhs) const {
				return _iterator != rhs._iterator;
			}

			PBOS_FORCEINLINE iterator &operator++() {
				++_iterator;
				return *this;
			}

			PBOS_FORCEINLINE iterator operator++(int) {
				iterator it = *this;
				++*this;
				return it;
			}

			PBOS_FORCEINLINE iterator next() {
				iterator iterator = *this;

				return ++iterator;
			}

			PBOS_FORCEINLINE iterator &operator--() {
				--_iterator;
				return *this;
			}

			PBOS_FORCEINLINE iterator operator--(int) {
				iterator it = *this;
				--*this;
				return it;
			}

			PBOS_FORCEINLINE iterator prev() {
				iterator iterator = *this;

				return --iterator;
			}

			PBOS_FORCEINLINE T &operator*() const {
				return *_iterator;
			}

			PBOS_FORCEINLINE T *operator->() const {
				return &*_iterator;
			}
		};

		PBOS_FORCEINLINE iterator begin() {
			return iterator(_tree.begin());
		}
		PBOS_FORCEINLINE iterator end() {
			return iterator(_tree.end());
		}
		PBOS_FORCEINLINE iterator begin_reversed() {
			return iterator(_tree.begin_reversed());
		}
		PBOS_FORCEINLINE iterator end_reversed() {
			return iterator(_tree.end_reversed());
		}

		struct const_iterator {
			iterator _iterator;
			PBOS_FORCEINLINE const_iterator(iterator &&iterator_in) : _iterator(iterator_in) {
			}
			const_iterator(const const_iterator &rhs) = default;
			const_iterator(const_iterator &&rhs) = default;
			const_iterator &operator=(const const_iterator &rhs) = default;
			const_iterator &operator=(const_iterator &&rhs) = default;

			PBOS_FORCEINLINE bool operator==(const const_iterator &rhs) const {
				return _iterator == rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator==(const_iterator &&rhs) const {
				return _iterator == rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(const const_iterator &rhs) const {
				return _iterator != rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(const_iterator &&rhs) const {
				return _iterator != rhs._iterator;
			}

			PBOS_FORCEINLINE const_iterator &operator++() {
				++_iterator;
				return *this;
			}

			PBOS_FORCEINLINE const_iterator operator++(int) {
				return _iterator++;
			}

			PBOS_FORCEINLINE const_iterator &operator--() {
				--_iterator;
				return *this;
			}

			PBOS_FORCEINLINE const_iterator operator--(int) {
				return _iterator--;
			}

			PBOS_FORCEINLINE const_iterator next() {
				const_iterator iterator = *this;

				return ++iterator;
			}

			PBOS_FORCEINLINE const_iterator prev() {
				const_iterator iterator = *this;

				return --iterator;
			}

			PBOS_FORCEINLINE const T &operator*() const {
				return *_iterator;
			}

			PBOS_FORCEINLINE const T &operator->() const {
				return *_iterator;
			}
		};

		PBOS_FORCEINLINE const_iterator begin() const noexcept {
			return const_iterator(const_cast<this_t *>(this)->begin());
		}
		PBOS_FORCEINLINE const_iterator end() const noexcept {
			return const_iterator(const_cast<this_t *>(this)->end());
		}
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

		PBOS_FORCEINLINE contains_result_t contains(const T &key) const {
			return contains_alt<T>(key);
		}

		template <typename U>
		PBOS_FORCEINLINE contains_result_t contains_alt(const U &key) const {
			if constexpr (Fallible) {
				auto node = _tree.template find_alt<U>(key);

				if (!node.has_value())
					return NULL_OPTION;

				return node.value();
			} else {
				return _tree.template find_alt<U>(key);
			}
		}
		PBOS_FORCEINLINE const_iterator find(const T &key) const {
			return const_cast<this_t *>(this)->find(key);
		}
		template <typename U>
		PBOS_FORCEINLINE const_iterator find_alt(const U &key) const {
			return const_cast<this_t *>(this)->find_alt<U>(key);
		}

		PBOS_FORCEINLINE iterator find(const T &key) {
			return find_alt<T>(key);
		}
		template <typename U>
		PBOS_FORCEINLINE iterator find_alt(const U &key) {
			if constexpr (Fallible) {
				if (auto node = _tree.template find_alt<U>(key); node.has_value()) {
					return iterator(typename tree_t::iterator(node.value(), &_tree, iterator_direction::Forward));
				}
				return _tree.end();
			} else {
				if (auto node = _tree.template find_alt<U>(key); node) {
					return iterator(typename tree_t::iterator(node, &_tree, iterator_direction::Forward));
				}
				return _tree.end();
			}
		}

		PBOS_FORCEINLINE iterator find_max_lteq(const T &key) {
			return find_max_lteq_alt<T>(key);
		}

		template <typename U>
		PBOS_FORCEINLINE iterator find_max_lteq_alt(const U &key) {
			if (auto node = _tree.template find_max_lteq_alt<U>(key); node) {
				return iterator(typename tree_t::iterator(node, &_tree, iterator_direction::Forward));
			}
			return _tree.end();
		}

		PBOS_FORCEINLINE const_iterator find_max_lteq(const T &key) const {
			return const_cast<this_t *>(this)->find_max_lteq(key);
		}

		template <typename U>
		PBOS_FORCEINLINE const_iterator find_max_lteq_alt(const U &key) const {
			return const_cast<this_t *>(this)->find_max_lteq_alt(key);
		}

		PBOS_FORCEINLINE kfxx::option_t<T> remove(const iterator &iterator) {
			return _tree.remove(iterator._iterator);
		}
	};

	template <typename T, typename Comparator = std::less<T>, bool IsThreeway = false>
	using set_t = _set_impl<T, Comparator, false, IsThreeway>;
	template <typename T, typename Comparator = fallible_less<T>, bool IsThreeway = false>
	using fallible_set_t = _set_impl<T, Comparator, true, IsThreeway>;
}

#endif
