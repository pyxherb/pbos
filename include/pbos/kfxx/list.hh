#ifndef _PBOS_KFXX_LIST_HH_
#define _PBOS_KFXX_LIST_HH_

#include "allocator.hh"
#include "basedefs.hh"
#include "rcobj.hh"
#include "scope_guard.hh"
#include "utils.hh"

namespace kfxx {
	template <typename T>
	class List {
	public:
		struct Node {
			Node *prev = nullptr, *next = nullptr;
			T data;

			PBOS_FORCEINLINE Node(T &&data) : data(std::move(data)) {
			}
		};

		using NodeHandle = Node *;

		static PBOS_FORCEINLINE NodeHandle null_node_handle() {
			return nullptr;
		}

	private:
		using ThisType = List<T>;

		Node *_first = nullptr, *_last = nullptr;
		size_t _length = 0;
		RcObjectPtr<Alloc> _allocator;

		[[nodiscard]] PBOS_FORCEINLINE Node *_alloc_node(T &&data) {
			Node *node = (Node *)_allocator->alloc(sizeof(Node), alignof(Node));
			if (!node)
				return nullptr;

			ScopeGuard ScopeGuard([this, node]() noexcept {
				_allocator->release(node, sizeof(Node), alignof(Node));
			});
			construct_at<Node>(node, std::move(data));
			ScopeGuard.release();

			return node;
		}

		PBOS_FORCEINLINE void _delete_node(Node *node) {
			std::destroy_at<Node>(node);

			_allocator->release(node, sizeof(Node), alignof(Node));
		}

		PBOS_FORCEINLINE void _prepend(Node *dest, Node *node) noexcept {
			if (dest) {
				if (dest->prev)
					dest->prev->next = node;
				dest->prev = node;

				if (dest == _first)
					_first = node;
			}
			node->next = dest;

			if (!_first)
				_first = node;
			if (!_last)
				_last = node;

			++_length;
		}

		PBOS_FORCEINLINE void _append(Node *dest, Node *node) noexcept {
			if (dest) {
				if (dest->next)
					dest->next->prev = node;
				dest->next = node;

				if (dest == _last)
					_last = node;
			}
			node->prev = dest;

			if (!_first)
				_first = node;
			if (!_last)
				_last = node;

			++_length;
		}

		PBOS_FORCEINLINE void _remove(Node *dest) {
			kd_assert(dest);

			if (dest == _first) {
				_first = dest->next;
			}
			if (dest == _last) {
				_last = dest->prev;
			}

			if (dest->prev)
				dest->prev->next = dest->next;
			if (dest->next)
				dest->next->prev = dest->prev;

			--_length;
		}

	public:
		PBOS_FORCEINLINE List(Alloc *allocator) : _allocator(allocator) {}
		List(const ThisType &other) = delete;
		PBOS_FORCEINLINE List(ThisType &&other) : _first(other._first), _last(other._last), _length(other._length), _allocator(std::move(other._allocator)) {
			other._first = nullptr;
			other._last = nullptr;
			other._length = 0;
		}
		PBOS_FORCEINLINE ThisType &operator=(ThisType &&other) {
			clear();
			construct_at<List>(this, std::move(other));
			return *this;
		}
		PBOS_FORCEINLINE ThisType &operator=(const ThisType &other) = delete;
		PBOS_FORCEINLINE ~List() {
			for (Node *i = _first; i != nullptr;) {
				Node *next = i->next;

				_delete_node(i);

				i = next;
			}
		}

		[[nodiscard]] PBOS_FORCEINLINE NodeHandle insert_front(NodeHandle node, NodeHandle new_node) {
			kd_assert(node);

			_prepend(node, new_node);

			return new_node;
		}

		[[nodiscard]] PBOS_FORCEINLINE NodeHandle insert_back(NodeHandle node, NodeHandle new_node) {
			kd_assert(node);

			_append(node, new_node);

			return new_node;
		}

		PBOS_FORCEINLINE void push_front(NodeHandle node) noexcept {
			_prepend(_first, node);
		}

		[[nodiscard]] PBOS_FORCEINLINE NodeHandle push_front(T &&data) {
			Node *new_node = _alloc_node(std::move(data));
			if (!new_node)
				return nullptr;
			_prepend(_first, new_node);
			return new_node;
		}

		PBOS_FORCEINLINE void push_back(NodeHandle node) noexcept {
			_append(_last, node);
		}

		[[nodiscard]] PBOS_FORCEINLINE NodeHandle push_back(T &&data) {
			Node *new_node = _alloc_node(std::move(data));
			if (!new_node)
				return nullptr;
			_append(_last, new_node);
			return new_node;
		}

		PBOS_FORCEINLINE void pop_front() {
			remove(_first);
		}

		PBOS_FORCEINLINE void pop_back() {
			remove(_last);
		}

		PBOS_FORCEINLINE void remove(NodeHandle node) {
			_remove(node);
			_delete_node(node);
		}

		PBOS_FORCEINLINE void detach(NodeHandle node) {
			_remove(node);
		}

		PBOS_FORCEINLINE static Node *next(NodeHandle cur_node, size_t index) {
			while (index) {
				kd_assert(cur_node);
				cur_node = cur_node->next;
				--index;
			}

			return cur_node;
		}

		PBOS_FORCEINLINE static Node *prev(NodeHandle cur_node, size_t index) {
			while (index) {
				kd_assert(cur_node);
				cur_node = cur_node->prev;
				--index;
			}

			return cur_node;
		}

		PBOS_FORCEINLINE void delete_node(NodeHandle node) {
			_delete_node(node);
		}

		PBOS_FORCEINLINE NodeHandle first_node() {
			return _first;
		}

		PBOS_FORCEINLINE const NodeHandle first_node() const {
			return _first;
		}

		PBOS_FORCEINLINE NodeHandle last_node() {
			return _last;
		}

		PBOS_FORCEINLINE const NodeHandle last_node() const {
			return _last;
		}

		PBOS_FORCEINLINE T &front() {
			return _first->data;
		}

		PBOS_FORCEINLINE const T &front() const {
			return _first->data;
		}

		PBOS_FORCEINLINE T &back() {
			return _last->data;
		}

		PBOS_FORCEINLINE const T &back() const {
			return _last->data;
		}

		PBOS_FORCEINLINE void clear() {
			for (Node *i = _first; i;) {
				Node *next_node = i->next;

				_delete_node(i);

				i = next_node;
			}
			_length = 0;
			_first = nullptr;
			_last = nullptr;
		}

		PBOS_FORCEINLINE size_t size() const {
			return _length;
		}

		struct Iterator {
			Node *node;
			ThisType *list;
			IteratorDirection direction;

			PBOS_FORCEINLINE Iterator(
				Node *node,
				ThisType *list,
				IteratorDirection direction)
				: node(node),
				  list(list),
				  direction(direction) {}

			Iterator(const Iterator &it) = default;
			PBOS_FORCEINLINE Iterator(Iterator &&it) {
				node = it.node;
				list = it.list;
				direction = it.direction;

				it.node = nullptr;
				it.list = nullptr;
				it.direction = IteratorDirection::Invalid;
			}
			PBOS_FORCEINLINE Iterator &operator=(const Iterator &rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible Iterator direction");
				node = rhs.node;
				list = rhs.list;
				return *this;
			}
			PBOS_FORCEINLINE Iterator &operator=(const Iterator &&rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible Iterator direction");
				node = rhs.node;
				list = rhs.list;
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
					node = node->next;
				} else {
					node = node->prev;
				}

				return *this;
			}

			PBOS_FORCEINLINE Iterator operator++(int) {
				Iterator it = *this;
				++(*this);
				return it;
			}

			PBOS_FORCEINLINE Iterator &operator--() {
				if (direction == IteratorDirection::Forward) {
					if (!(node = node->prev))
						km_panic("Dereasing the begin Iterator");
				} else {
					if (!(node = node->next))
						km_panic("Dereasing the begin Iterator");
				}

				return *this;
			}

			PBOS_FORCEINLINE Iterator operator--(int) {
				Iterator it = *this;
				--(*this);
				return it;
			}

			PBOS_FORCEINLINE bool operator==(const Node *node) const noexcept {
				return node == node;
			}

			PBOS_FORCEINLINE bool operator==(const Iterator &it) const {
				if (list != it.list)
					km_panic("Cannot compare iterators from different lists");
				return node == it.node;
			}

			PBOS_FORCEINLINE bool operator==(const Iterator &&rhs) const {
				const Iterator it = rhs;
				return *this == it;
			}

			PBOS_FORCEINLINE bool operator!=(const Node *node) const noexcept {
				return node != node;
			}

			PBOS_FORCEINLINE bool operator!=(const Iterator &it) const {
				if (list != it.list)
					km_panic("Cannot compare iterators from different lists");
				return node != it.node;
			}

			PBOS_FORCEINLINE bool operator!=(Iterator &&rhs) const {
				Iterator it = rhs;
				return *this != it;
			}

			PBOS_FORCEINLINE T &operator*() {
				if (!node)
					km_panic("Deferencing the end Iterator");
				return node->data;
			}

			PBOS_FORCEINLINE T &operator*() const {
				if (!node)
					km_panic("Deferencing the end Iterator");
				return node->data;
			}

			PBOS_FORCEINLINE T *operator->() {
				if (!node)
					km_panic("Deferencing the end Iterator");
				return &node->data;
			}

			PBOS_FORCEINLINE T *operator->() const {
				if (!node)
					km_panic("Deferencing the end Iterator");
				return &node->data;
			}
		};

		PBOS_FORCEINLINE Iterator begin() {
			return Iterator(_first, this, IteratorDirection::Forward);
		}
		PBOS_FORCEINLINE Iterator end() {
			return Iterator(nullptr, this, IteratorDirection::Forward);
		}
		PBOS_FORCEINLINE Iterator begin_reversed() {
			return Iterator(_last, this, IteratorDirection::Reversed);
		}
		PBOS_FORCEINLINE Iterator end_reversed() {
			return Iterator(nullptr, this, IteratorDirection::Reversed);
		}

		struct ConstIterator {
			const Node *node;
			const List<T> *list;
			IteratorDirection direction;

			PBOS_FORCEINLINE ConstIterator(
				const Node *node,
				const List<T> *list,
				IteratorDirection direction)
				: node(node),
				  list(list),
				  direction(direction) {}

			ConstIterator(const ConstIterator &it) = default;
			PBOS_FORCEINLINE ConstIterator(ConstIterator &&it) {
				node = it.node;
				list = it.list;
				direction = it.direction;

				it.node = nullptr;
				it.list = nullptr;
				it.direction = IteratorDirection::Invalid;
			}
			PBOS_FORCEINLINE ConstIterator &operator=(const ConstIterator &rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible Iterator direction");
				construct_at<ConstIterator>(this, rhs);
				return *this;
			}
			PBOS_FORCEINLINE ConstIterator &operator=(ConstIterator &&rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible Iterator direction");
				construct_at<ConstIterator>(this, std::move(rhs));
				return *this;
			}

			PBOS_FORCEINLINE ConstIterator(const Iterator &it) {
				(*this) = it;
			}
			PBOS_FORCEINLINE ConstIterator(Iterator &&it) {
				(*this) = it;
			}
			PBOS_FORCEINLINE ConstIterator &operator=(const Iterator &rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible Iterator direction");
				node = rhs.node;
				list = rhs.list;
				return *this;
			}
			PBOS_FORCEINLINE ConstIterator &operator=(Iterator &&rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible Iterator direction");
				node = rhs.node;
				list = rhs.list;
				return *this;
			}

			PBOS_FORCEINLINE bool copy(ConstIterator &dest) noexcept {
				dest = *this;
				return true;
			}

			PBOS_FORCEINLINE ConstIterator &operator++() {
				if (!node)
					km_panic("Increasing the end Iterator");

				if (direction == IteratorDirection::Forward) {
					node = node->next;
				} else {
					node = node->prev;
				}

				return *this;
			}

			PBOS_FORCEINLINE ConstIterator operator++(int) {
				ConstIterator it = *this;
				++(*this);
				return it;
			}

			PBOS_FORCEINLINE ConstIterator &operator--() {
				if (direction == IteratorDirection::Forward) {
					if (!(node = node->prev))
						km_panic("Dereasing the begin Iterator");
				} else {
					if (!(node = node->next))
						km_panic("Dereasing the begin Iterator");
				}

				return *this;
			}

			PBOS_FORCEINLINE ConstIterator operator--(int) {
				ConstIterator it = *this;
				--(*this);
				return it;
			}

			PBOS_FORCEINLINE bool operator==(const Node *node) const noexcept {
				return node == node;
			}

			PBOS_FORCEINLINE bool operator==(const ConstIterator &it) const {
				if (list != it.list)
					km_panic("Cannot compare iterators from different lists");
				return node == it.node;
			}

			PBOS_FORCEINLINE bool operator==(ConstIterator &&rhs) const {
				const ConstIterator it = rhs;
				return *this == it;
			}

			PBOS_FORCEINLINE bool operator!=(const Node *node) const noexcept {
				return node != node;
			}

			PBOS_FORCEINLINE bool operator!=(const ConstIterator &it) const {
				if (list != it.list)
					km_panic("Cannot compare iterators from different lists");
				return node != it.node;
			}

			PBOS_FORCEINLINE bool operator!=(ConstIterator &&rhs) const {
				ConstIterator it = rhs;
				return *this != it;
			}

			PBOS_FORCEINLINE const T &operator*() {
				if (!node)
					km_panic("Deferencing the end Iterator");
				return node->data;
			}

			PBOS_FORCEINLINE const T &operator*() const {
				if (!node)
					km_panic("Deferencing the end Iterator");
				return node->data;
			}

			PBOS_FORCEINLINE const T *operator->() {
				if (!node)
					km_panic("Deferencing the end Iterator");
				return &node->data;
			}

			PBOS_FORCEINLINE const T *operator->() const {
				if (!node)
					km_panic("Deferencing the end Iterator");
				return &node->data;
			}
		};

		PBOS_FORCEINLINE ConstIterator begin_const() const noexcept {
			return ConstIterator((Node *)_first, this, IteratorDirection::Forward);
		}
		PBOS_FORCEINLINE ConstIterator end_const() const noexcept {
			return ConstIterator(nullptr, this, IteratorDirection::Forward);
		}
		PBOS_FORCEINLINE ConstIterator begin_const_reversed() const noexcept {
			return ConstIterator((Node *)_last, this, IteratorDirection::Reversed);
		}
		PBOS_FORCEINLINE ConstIterator end_const_reversed() const noexcept {
			return ConstIterator(nullptr, this, IteratorDirection::Reversed);
		}

		PBOS_FORCEINLINE ConstIterator begin() const noexcept {
			return begin_const();
		}
		PBOS_FORCEINLINE ConstIterator end() const noexcept {
			return end_const();
		}

		PBOS_FORCEINLINE Alloc *allocator() const {
			return _allocator.get();
		}
	};
}

#endif
