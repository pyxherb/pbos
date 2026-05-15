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
	class SetImpl final {
	private:
		static_assert(std::is_move_constructible_v<T>, "The element must be move-constructible");
		using tree_t = std::conditional_t<Fallible, FallibleRBTree<T, Comparator, IsThreeway>, RBTree<T, Comparator, IsThreeway>>;
		tree_t _tree;
		kfxx::RcObjectPtr<kfxx::Allocator> _allocator;
		using ThisType = kfxx::SetImpl<T, Comparator, Fallible, IsThreeway>;

	public:
		using RemoveResultType = typename std::conditional_t<Fallible, bool, void>;
		using element_query_result_t = typename std::conditional_t<Fallible, Option<T &>, T &>;
		using const_element_query_result_t = typename std::conditional_t<Fallible, Option<const T &>, const T &>;
		using ContainsResultType = typename std::conditional_t<Fallible, Option<bool>, bool>;

		using node_t = typename tree_t::node_t;

		PBOS_FORCEINLINE SetImpl(Allocator *allocator, Comparator &&comparator = {}) noexcept : _allocator(allocator), _tree(std::move(comparator)) {
		}
		PBOS_FORCEINLINE SetImpl(ThisType &&rhs) noexcept : _tree(std::move(rhs._tree)) {
		}
		PBOS_FORCEINLINE ~SetImpl() {
		}

		PBOS_FORCEINLINE ThisType &operator=(ThisType &&dest) noexcept {
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

		PBOS_FORCEINLINE Allocator *allocator() const {
			return _allocator.get();
		}

		PBOS_FORCEINLINE void replace_allocator(Allocator *rhs) noexcept {
			_tree.replace_allocator(rhs);
		}

		PBOS_FORCEINLINE Comparator &comparator() {
			return _tree.comparator();
		}

		PBOS_FORCEINLINE const Comparator &comparator() const {
			return _tree.comparator();
		}

		PBOS_FORCEINLINE void clear() {
			_tree.clear([this](tree_t::node_t *node) {
				kfxx::destroy_and_release<ThisType::node_t>(_allocator.get(), node);
			});
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

				kd_assert(node.value());

				return node.value()->value;
			} else {
				auto node = _tree.template find_alt<U>(key);

				kd_assert(node);

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

				kd_assert(node.value());

				return node.value()->value;
			} else {
				auto node = _tree.template find_alt<U>(key);

				kd_assert(node);

				return node->rb_value;
			}
		}

		struct Iterator {
			typename tree_t::Iterator _iterator;
			PBOS_FORCEINLINE Iterator(typename tree_t::Iterator &&iterator_in) : _iterator(iterator_in) {
			}
			Iterator(const Iterator &rhs) = default;
			Iterator(Iterator &&rhs) = default;
			Iterator &operator=(const Iterator &rhs) = default;
			Iterator &operator=(Iterator &&rhs) = default;

			PBOS_FORCEINLINE bool operator==(const Iterator &rhs) const {
				return _iterator == rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator==(Iterator &&rhs) const {
				return _iterator == rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(const Iterator &rhs) const {
				return _iterator != rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(Iterator &&rhs) const {
				return _iterator != rhs._iterator;
			}

			PBOS_FORCEINLINE Iterator &operator++() {
				++_iterator;
				return *this;
			}

			PBOS_FORCEINLINE Iterator operator++(int) {
				Iterator it = *this;
				++*this;
				return it;
			}

			PBOS_FORCEINLINE Iterator next() {
				Iterator Iterator = *this;

				return ++Iterator;
			}

			PBOS_FORCEINLINE Iterator &operator--() {
				--_iterator;
				return *this;
			}

			PBOS_FORCEINLINE Iterator operator--(int) {
				Iterator it = *this;
				--*this;
				return it;
			}

			PBOS_FORCEINLINE Iterator prev() {
				Iterator Iterator = *this;

				return --Iterator;
			}

			PBOS_FORCEINLINE T &operator*() const {
				return *_iterator;
			}

			PBOS_FORCEINLINE T *operator->() const {
				return &*_iterator;
			}
		};

		PBOS_FORCEINLINE Iterator begin() {
			return Iterator(_tree.begin());
		}
		PBOS_FORCEINLINE Iterator end() {
			return Iterator(_tree.end());
		}
		PBOS_FORCEINLINE Iterator begin_reversed() {
			return Iterator(_tree.begin_reversed());
		}
		PBOS_FORCEINLINE Iterator end_reversed() {
			return Iterator(_tree.end_reversed());
		}

		struct ConstIterator {
			Iterator _iterator;
			PBOS_FORCEINLINE ConstIterator(Iterator &&iterator_in) : _iterator(iterator_in) {
			}
			ConstIterator(const ConstIterator &rhs) = default;
			ConstIterator(ConstIterator &&rhs) = default;
			ConstIterator &operator=(const ConstIterator &rhs) = default;
			ConstIterator &operator=(ConstIterator &&rhs) = default;

			PBOS_FORCEINLINE bool operator==(const ConstIterator &rhs) const {
				return _iterator == rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator==(ConstIterator &&rhs) const {
				return _iterator == rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(const ConstIterator &rhs) const {
				return _iterator != rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(ConstIterator &&rhs) const {
				return _iterator != rhs._iterator;
			}

			PBOS_FORCEINLINE ConstIterator &operator++() {
				++_iterator;
				return *this;
			}

			PBOS_FORCEINLINE ConstIterator operator++(int) {
				return _iterator++;
			}

			PBOS_FORCEINLINE ConstIterator &operator--() {
				--_iterator;
				return *this;
			}

			PBOS_FORCEINLINE ConstIterator operator--(int) {
				return _iterator--;
			}

			PBOS_FORCEINLINE ConstIterator next() {
				ConstIterator Iterator = *this;

				return ++Iterator;
			}

			PBOS_FORCEINLINE ConstIterator prev() {
				ConstIterator Iterator = *this;

				return --Iterator;
			}

			PBOS_FORCEINLINE const T &operator*() const {
				return *_iterator;
			}

			PBOS_FORCEINLINE const T &operator->() const {
				return *_iterator;
			}
		};

		PBOS_FORCEINLINE ConstIterator begin() const noexcept {
			return ConstIterator(const_cast<ThisType *>(this)->begin());
		}
		PBOS_FORCEINLINE ConstIterator end() const noexcept {
			return ConstIterator(const_cast<ThisType *>(this)->end());
		}
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

		PBOS_FORCEINLINE ContainsResultType contains(const T &key) const {
			return contains_alt<T>(key);
		}

		template <typename U>
		PBOS_FORCEINLINE ContainsResultType contains_alt(const U &key) const {
			if constexpr (Fallible) {
				auto node = _tree.template find_alt<U>(key);

				if (!node.has_value())
					return NULL_OPTION;

				return node.value();
			} else {
				return _tree.template find_alt<U>(key);
			}
		}
		PBOS_FORCEINLINE ConstIterator find(const T &key) const {
			return const_cast<ThisType *>(this)->find(key);
		}
		template <typename U>
		PBOS_FORCEINLINE ConstIterator find_alt(const U &key) const {
			return const_cast<ThisType *>(this)->find_alt<U>(key);
		}

		PBOS_FORCEINLINE Iterator find(const T &key) {
			return find_alt<T>(key);
		}
		template <typename U>
		PBOS_FORCEINLINE Iterator find_alt(const U &key) {
			if constexpr (Fallible) {
				if (auto node = _tree.template find_alt<U>(key); node.has_value()) {
					return Iterator(typename tree_t::Iterator(node.value(), &_tree, IteratorDirection::Forward));
				}
				return _tree.end();
			} else {
				if (auto node = _tree.template find_alt<U>(key); node) {
					return Iterator(typename tree_t::Iterator(node, &_tree, IteratorDirection::Forward));
				}
				return _tree.end();
			}
		}

		PBOS_FORCEINLINE Iterator find_max_lteq(const T &key) {
			return find_max_lteq_alt<T>(key);
		}

		template <typename U>
		PBOS_FORCEINLINE Iterator find_max_lteq_alt(const U &key) {
			if (auto node = _tree.template find_max_lteq_alt<U>(key); node) {
				return Iterator(typename tree_t::Iterator(node, &_tree, IteratorDirection::Forward));
			}
			return _tree.end();
		}

		PBOS_FORCEINLINE ConstIterator find_max_lteq(const T &key) const {
			return const_cast<ThisType *>(this)->find_max_lteq(key);
		}

		template <typename U>
		PBOS_FORCEINLINE ConstIterator find_max_lteq_alt(const U &key) const {
			return const_cast<ThisType *>(this)->find_max_lteq_alt(key);
		}

		PBOS_FORCEINLINE void remove(const Iterator &Iterator) {
			_tree.remove(Iterator._iterator);
		}
	};

	template <typename T, typename Comparator = std::less<T>, bool IsThreeway = false>
	using Set = SetImpl<T, Comparator, false, IsThreeway>;
	template <typename T, typename Comparator = FallibleLt<T>, bool IsThreeway = false>
	using FallibleSet = SetImpl<T, Comparator, true, IsThreeway>;
}

#endif
