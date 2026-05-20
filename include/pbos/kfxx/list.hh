#ifndef _PBOS_KFXX_LIST_HH_
#define _PBOS_KFXX_LIST_HH_

#include "allocator.hh"
#include "basedefs.hh"
#include "rcobj.hh"
#include "scope_guard.hh"
#include "utils.hh"

namespace kfxx {
	template <typename T>
	class list_t {
	public:
		struct node_t {
			node_t *prev = nullptr, *next = nullptr;
			T data;

			PBOS_FORCEINLINE node_t(T &&data) : data(std::move(data)) {
			}
		};

		using NodeHandle = node_t *;

		static PBOS_FORCEINLINE NodeHandle null_node_handle() {
			return nullptr;
		}

	private:
		using ThisType = list_t<T>;

		node_t *_first = nullptr, *_last = nullptr;
		size_t _length = 0;
		rc_object_ptr<allocator_t> _allocator;

		[[nodiscard]] PBOS_FORCEINLINE node_t *_alloc_node(T &&data) {
			node_t *node = (node_t *)_allocator->alloc(sizeof(node_t), alignof(node_t));
			if (!node)
				return nullptr;

			scope_guard scope_guard([this, node]() noexcept {
				_allocator->release(node, sizeof(node_t), alignof(node_t));
			});
			construct_at<node_t>(node, std::move(data));
			scope_guard.release();

			return node;
		}

		PBOS_FORCEINLINE void _delete_node(node_t *node) {
			std::destroy_at<node_t>(node);

			_allocator->release(node, sizeof(node_t), alignof(node_t));
		}

		PBOS_FORCEINLINE void _prepend(node_t *dest, node_t *node) noexcept {
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

		PBOS_FORCEINLINE void _append(node_t *dest, node_t *node) noexcept {
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

		PBOS_FORCEINLINE void _remove(node_t *dest) {
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
		PBOS_FORCEINLINE list_t(allocator_t *allocator) : _allocator(allocator) {}
		list_t(const ThisType &other) = delete;
		PBOS_FORCEINLINE list_t(ThisType &&other) : _first(other._first), _last(other._last), _length(other._length), _allocator(std::move(other._allocator)) {
			other._first = nullptr;
			other._last = nullptr;
			other._length = 0;
		}
		PBOS_FORCEINLINE ThisType &operator=(ThisType &&other) {
			clear();
			construct_at<list_t>(this, std::move(other));
			return *this;
		}
		PBOS_FORCEINLINE ThisType &operator=(const ThisType &other) = delete;
		PBOS_FORCEINLINE ~list_t() {
			for (node_t *i = _first; i != nullptr;) {
				node_t *next = i->next;

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
			node_t *new_node = _alloc_node(std::move(data));
			if (!new_node)
				return nullptr;
			_prepend(_first, new_node);
			return new_node;
		}

		PBOS_FORCEINLINE void push_back(NodeHandle node) noexcept {
			_append(_last, node);
		}

		[[nodiscard]] PBOS_FORCEINLINE NodeHandle push_back(T &&data) {
			node_t *new_node = _alloc_node(std::move(data));
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

		PBOS_FORCEINLINE static node_t *next(NodeHandle cur_node, size_t index) {
			while (index) {
				kd_assert(cur_node);
				cur_node = cur_node->next;
				--index;
			}

			return cur_node;
		}

		PBOS_FORCEINLINE static node_t *prev(NodeHandle cur_node, size_t index) {
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
			for (node_t *i = _first; i;) {
				node_t *next_node = i->next;

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

		struct iterator {
			node_t *node;
			ThisType *list;
			iteratorDirection direction;

			PBOS_FORCEINLINE iterator(
				node_t *node,
				ThisType *list,
				iteratorDirection direction)
				: node(node),
				  list(list),
				  direction(direction) {}

			iterator(const iterator &it) = default;
			PBOS_FORCEINLINE iterator(iterator &&it) {
				node = it.node;
				list = it.list;
				direction = it.direction;

				it.node = nullptr;
				it.list = nullptr;
				it.direction = iteratorDirection::Invalid;
			}
			PBOS_FORCEINLINE iterator &operator=(const iterator &rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible iterator direction");
				node = rhs.node;
				list = rhs.list;
				return *this;
			}
			PBOS_FORCEINLINE iterator &operator=(const iterator &&rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible iterator direction");
				node = rhs.node;
				list = rhs.list;
				return *this;
			}

			PBOS_FORCEINLINE bool copy(iterator &dest) noexcept {
				dest = *this;
				return true;
			}

			PBOS_FORCEINLINE iterator &operator++() {
				if (!node)
					km_panic("Increasing the end iterator");

				if (direction == iteratorDirection::Forward) {
					node = node->next;
				} else {
					node = node->prev;
				}

				return *this;
			}

			PBOS_FORCEINLINE iterator operator++(int) {
				iterator it = *this;
				++(*this);
				return it;
			}

			PBOS_FORCEINLINE iterator &operator--() {
				if (direction == iteratorDirection::Forward) {
					if (!(node = node->prev))
						km_panic("Dereasing the begin iterator");
				} else {
					if (!(node = node->next))
						km_panic("Dereasing the begin iterator");
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
				if (list != it.list)
					km_panic("Cannot compare iterators from different lists");
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
				if (list != it.list)
					km_panic("Cannot compare iterators from different lists");
				return node != it.node;
			}

			PBOS_FORCEINLINE bool operator!=(iterator &&rhs) const {
				iterator it = rhs;
				return *this != it;
			}

			PBOS_FORCEINLINE T &operator*() {
				if (!node)
					km_panic("Deferencing the end iterator");
				return node->data;
			}

			PBOS_FORCEINLINE T &operator*() const {
				if (!node)
					km_panic("Deferencing the end iterator");
				return node->data;
			}

			PBOS_FORCEINLINE T *operator->() {
				if (!node)
					km_panic("Deferencing the end iterator");
				return &node->data;
			}

			PBOS_FORCEINLINE T *operator->() const {
				if (!node)
					km_panic("Deferencing the end iterator");
				return &node->data;
			}
		};

		PBOS_FORCEINLINE iterator begin() {
			return iterator(_first, this, iteratorDirection::Forward);
		}
		PBOS_FORCEINLINE iterator end() {
			return iterator(nullptr, this, iteratorDirection::Forward);
		}
		PBOS_FORCEINLINE iterator begin_reversed() {
			return iterator(_last, this, iteratorDirection::Reversed);
		}
		PBOS_FORCEINLINE iterator end_reversed() {
			return iterator(nullptr, this, iteratorDirection::Reversed);
		}

		struct const_iterator {
			const node_t *node;
			const list_t<T> *list;
			iteratorDirection direction;

			PBOS_FORCEINLINE const_iterator(
				const node_t *node,
				const list_t<T> *list,
				iteratorDirection direction)
				: node(node),
				  list(list),
				  direction(direction) {}

			const_iterator(const const_iterator &it) = default;
			PBOS_FORCEINLINE const_iterator(const_iterator &&it) {
				node = it.node;
				list = it.list;
				direction = it.direction;

				it.node = nullptr;
				it.list = nullptr;
				it.direction = iteratorDirection::Invalid;
			}
			PBOS_FORCEINLINE const_iterator &operator=(const const_iterator &rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible iterator direction");
				construct_at<const_iterator>(this, rhs);
				return *this;
			}
			PBOS_FORCEINLINE const_iterator &operator=(const_iterator &&rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible iterator direction");
				construct_at<const_iterator>(this, std::move(rhs));
				return *this;
			}

			PBOS_FORCEINLINE const_iterator(const iterator &it) {
				(*this) = it;
			}
			PBOS_FORCEINLINE const_iterator(iterator &&it) {
				(*this) = it;
			}
			PBOS_FORCEINLINE const_iterator &operator=(const iterator &rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible iterator direction");
				node = rhs.node;
				list = rhs.list;
				return *this;
			}
			PBOS_FORCEINLINE const_iterator &operator=(iterator &&rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible iterator direction");
				node = rhs.node;
				list = rhs.list;
				return *this;
			}

			PBOS_FORCEINLINE bool copy(const_iterator &dest) noexcept {
				dest = *this;
				return true;
			}

			PBOS_FORCEINLINE const_iterator &operator++() {
				if (!node)
					km_panic("Increasing the end iterator");

				if (direction == iteratorDirection::Forward) {
					node = node->next;
				} else {
					node = node->prev;
				}

				return *this;
			}

			PBOS_FORCEINLINE const_iterator operator++(int) {
				const_iterator it = *this;
				++(*this);
				return it;
			}

			PBOS_FORCEINLINE const_iterator &operator--() {
				if (direction == iteratorDirection::Forward) {
					if (!(node = node->prev))
						km_panic("Dereasing the begin iterator");
				} else {
					if (!(node = node->next))
						km_panic("Dereasing the begin iterator");
				}

				return *this;
			}

			PBOS_FORCEINLINE const_iterator operator--(int) {
				const_iterator it = *this;
				--(*this);
				return it;
			}

			PBOS_FORCEINLINE bool operator==(const node_t *node) const noexcept {
				return node == node;
			}

			PBOS_FORCEINLINE bool operator==(const const_iterator &it) const {
				if (list != it.list)
					km_panic("Cannot compare iterators from different lists");
				return node == it.node;
			}

			PBOS_FORCEINLINE bool operator==(const_iterator &&rhs) const {
				const const_iterator it = rhs;
				return *this == it;
			}

			PBOS_FORCEINLINE bool operator!=(const node_t *node) const noexcept {
				return node != node;
			}

			PBOS_FORCEINLINE bool operator!=(const const_iterator &it) const {
				if (list != it.list)
					km_panic("Cannot compare iterators from different lists");
				return node != it.node;
			}

			PBOS_FORCEINLINE bool operator!=(const_iterator &&rhs) const {
				const_iterator it = rhs;
				return *this != it;
			}

			PBOS_FORCEINLINE const T &operator*() {
				if (!node)
					km_panic("Deferencing the end iterator");
				return node->data;
			}

			PBOS_FORCEINLINE const T &operator*() const {
				if (!node)
					km_panic("Deferencing the end iterator");
				return node->data;
			}

			PBOS_FORCEINLINE const T *operator->() {
				if (!node)
					km_panic("Deferencing the end iterator");
				return &node->data;
			}

			PBOS_FORCEINLINE const T *operator->() const {
				if (!node)
					km_panic("Deferencing the end iterator");
				return &node->data;
			}
		};

		PBOS_FORCEINLINE const_iterator begin_const() const noexcept {
			return const_iterator((node_t *)_first, this, iteratorDirection::Forward);
		}
		PBOS_FORCEINLINE const_iterator end_const() const noexcept {
			return const_iterator(nullptr, this, iteratorDirection::Forward);
		}
		PBOS_FORCEINLINE const_iterator begin_const_reversed() const noexcept {
			return const_iterator((node_t *)_last, this, iteratorDirection::Reversed);
		}
		PBOS_FORCEINLINE const_iterator end_const_reversed() const noexcept {
			return const_iterator(nullptr, this, iteratorDirection::Reversed);
		}

		PBOS_FORCEINLINE const_iterator begin() const noexcept {
			return begin_const();
		}
		PBOS_FORCEINLINE const_iterator end() const noexcept {
			return end_const();
		}

		PBOS_FORCEINLINE allocator_t *allocator() const {
			return _allocator.get();
		}
	};
}

#endif
