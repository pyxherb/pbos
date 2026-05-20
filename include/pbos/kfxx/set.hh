#ifndef _PBOS_KFXX_SET_H_
#define _PBOS_KFXX_SET_H_

#include <concepts>
#include "allocator.hh"
#include "option.hh"
#include "rbtree.hh"
#include "rcobj.hh"
#include "scope_guard.hh"

namespace kfxx {
	template <typename T, typename Comparator, bool Fallible, bool IsThreeway>
	class set_impl final {
	private:
		static_assert(std::is_move_constructible_v<T>, "The element must be move-constructible");
		using Tree = std::conditional_t<Fallible, fallible_rbtree_t<T, Comparator, IsThreeway>, rbtree_t<T, Comparator, IsThreeway>>;
		Tree _tree;
		kfxx::rc_object_ptr<kfxx::allocator_t> _allocator;
		using ThisType = kfxx::set_impl<T, Comparator, Fallible, IsThreeway>;

	public:
		using RemoveResultType = typename std::conditional_t<Fallible, bool, void>;
		using ElementQueryResult = typename std::conditional_t<Fallible, option_t<T &>, T &>;
		using ConstElementQueryResult = typename std::conditional_t<Fallible, option_t<const T &>, const T &>;
		using ContainsResultType = typename std::conditional_t<Fallible, option_t<bool>, bool>;

		using Node = typename Tree::node_t;

		PBOS_FORCEINLINE set_impl(allocator_t *allocator, Comparator &&comparator = {}) noexcept : _allocator(allocator), _tree(std::move(comparator)) {
		}
		PBOS_FORCEINLINE set_impl(ThisType &&rhs) noexcept : _tree(std::move(rhs._tree)) {
		}
		PBOS_FORCEINLINE ~set_impl() {
		}

		PBOS_FORCEINLINE ThisType &operator=(ThisType &&dest) noexcept {
			_tree = std::move(dest._tree);
			return *this;
		}

		[[nodiscard]] PBOS_FORCEINLINE bool insert(T &&value) {
			typename Tree::node_t *node = kfxx::alloc_and_construct<Node>(allocator(), std::move(value));

			if (!_tree.insert(node)) {
				kfxx::destroy_and_release(allocator(), node);
				return false;
			}

			return true;
		}

		PBOS_FORCEINLINE RemoveResultType remove(const T &key) {
			return remove_alt<T>(key);
		}

		template <typename U>
		PBOS_FORCEINLINE RemoveResultType remove_alt(const U &key) {
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
			_tree.clear([this](Tree::node_t *node) {
				kfxx::destroy_and_release<ThisType::Node>(_allocator.get(), node);
			});
		}

		PBOS_FORCEINLINE ElementQueryResult at(const T &key) {
			return at_alt<T>(key);
		}

		template <typename U>
		PBOS_FORCEINLINE ElementQueryResult at_alt(const U &key) {
			if constexpr (Fallible) {
				auto node = _tree.template find_alt<U>(key);

				if (!node.has_value())
					return nullopt;

				kd_assert(node.value());

				return node.value()->value;
			} else {
				auto node = _tree.template find_alt<U>(key);

				kd_assert(node);

				return node->rb_value;
			}
		}

		PBOS_FORCEINLINE ConstElementQueryResult at(const T &key) const {
			return at_alt<T>(key);
		}

		template <typename U>
		PBOS_FORCEINLINE ConstElementQueryResult at_alt(const U &key) const {
			if constexpr (Fallible) {
				auto node = _tree.template find_alt<U>(key);

				if (!node.has_value())
					return nullopt;

				kd_assert(node.value());

				return node.value()->value;
			} else {
				auto node = _tree.template find_alt<U>(key);

				kd_assert(node);

				return node->rb_value;
			}
		}

		struct iterator {
			typename Tree::iterator _iterator;
			PBOS_FORCEINLINE iterator(typename Tree::iterator &&iterator_in) : _iterator(iterator_in) {
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
			return const_iterator(const_cast<ThisType *>(this)->begin());
		}
		PBOS_FORCEINLINE const_iterator end() const noexcept {
			return const_iterator(const_cast<ThisType *>(this)->end());
		}
		PBOS_FORCEINLINE const_iterator begin_const() const noexcept {
			return const_iterator(const_cast<ThisType *>(this)->begin());
		}
		PBOS_FORCEINLINE const_iterator end_const() const noexcept {
			return const_iterator(const_cast<ThisType *>(this)->end());
		}
		PBOS_FORCEINLINE const_iterator begin_const_reversed() const noexcept {
			return const_iterator(const_cast<ThisType *>(this)->begin_reversed());
		}
		PBOS_FORCEINLINE const_iterator end_const_reversed() const noexcept {
			return const_iterator(const_cast<ThisType *>(this)->end_reversed());
		}

		PBOS_FORCEINLINE ContainsResultType contains(const T &key) const {
			return contains_alt<T>(key);
		}

		template <typename U>
		PBOS_FORCEINLINE ContainsResultType contains_alt(const U &key) const {
			if constexpr (Fallible) {
				auto node = _tree.template find_alt<U>(key);

				if (!node.has_value())
					return nullopt;

				return node.value();
			} else {
				return _tree.template find_alt<U>(key);
			}
		}
		PBOS_FORCEINLINE const_iterator find(const T &key) const {
			return const_cast<ThisType *>(this)->find(key);
		}
		template <typename U>
		PBOS_FORCEINLINE const_iterator find_alt(const U &key) const {
			return const_cast<ThisType *>(this)->find_alt<U>(key);
		}

		PBOS_FORCEINLINE iterator find(const T &key) {
			return find_alt<T>(key);
		}
		template <typename U>
		PBOS_FORCEINLINE iterator find_alt(const U &key) {
			if constexpr (Fallible) {
				if (auto node = _tree.template find_alt<U>(key); node.has_value()) {
					return iterator(typename Tree::iterator(node.value(), &_tree, iteratorDirection::Forward));
				}
				return _tree.end();
			} else {
				if (auto node = _tree.template find_alt<U>(key); node) {
					return iterator(typename Tree::iterator(node, &_tree, iteratorDirection::Forward));
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
				return iterator(typename Tree::iterator(node, &_tree, iteratorDirection::Forward));
			}
			return _tree.end();
		}

		PBOS_FORCEINLINE const_iterator find_max_lteq(const T &key) const {
			return const_cast<ThisType *>(this)->find_max_lteq(key);
		}

		template <typename U>
		PBOS_FORCEINLINE const_iterator find_max_lteq_alt(const U &key) const {
			return const_cast<ThisType *>(this)->find_max_lteq_alt(key);
		}

		PBOS_FORCEINLINE void remove(const iterator &iterator) {
			_tree.remove(iterator._iterator);
		}
	};

	template <typename T, typename Comparator = std::less<T>, bool IsThreeway = false>
	using set_t = set_impl<T, Comparator, false, IsThreeway>;
	template <typename T, typename Comparator = fallible_less<T>, bool IsThreeway = false>
	using fallible_set = set_impl<T, Comparator, true, IsThreeway>;
}

#endif
