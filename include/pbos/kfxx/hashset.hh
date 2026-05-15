#ifndef _PBOS_KFXX_HASHSET_H_
#define _PBOS_KFXX_HASHSET_H_

#include "dynarray.hh"
#include "fallible_cmp.hh"
#include "fallible_hash.hh"
#include "hash.hh"
#include "list.hh"
#include "option.hh"

namespace kfxx {
	namespace details {
		template <typename T, typename V = void>
		struct HashCodeResultTypeExtractor {
			using type = T;
		};

		template <typename T>
		struct HashCodeResultTypeExtractor<T, std::void_t<decltype(std::declval<T>().value())>> {
			using type = typename T::value_type;
		};
	}

	template <
		typename T,
		typename EqCmp,
		typename Hasher,
		bool Fallible>
	// PBOS_REQUIRES_CONCEPT(std::invocable<EqCmp, const T &, const T &>)
	class HashSetImpl {
	public:
		static_assert(std::is_move_constructible_v<T>, "The element must be move-constructible");
		using hasher_result_t = decltype(std::declval<Hasher>()(std::declval<T>()));
		using hash_code_t = typename details::HashCodeResultTypeExtractor<hasher_result_t>::type;

		struct element_t {
			T data;
			hash_code_t hash_code;

			PBOS_FORCEINLINE element_t(T &&data, hash_code_t hash_code) : data(std::move(data)), hash_code(hash_code) {}

			element_t(element_t &&rhs) = default;
			element_t &operator=(element_t &&rhs) = default;
		};

		using bucket_t = List<element_t>;

	public:
		using RemoveResultType = typename std::conditional_t<Fallible, bool, void>;
		using element_query_result_t = typename std::conditional_t<Fallible, Option<T &>, T &>;
		using bucket_node_handle_query_result_t = typename std::conditional_t<Fallible, Option<typename bucket_t::NodeHandle>, typename bucket_t::NodeHandle>;
		using const_element_query_result_t = typename std::conditional_t<Fallible, Option<const T &>, const T &>;
		using ContainsResultType = typename std::conditional_t<Fallible, Option<bool>, bool>;

	private:
		using ThisType = HashSetImpl<T, EqCmp, Hasher, Fallible>;

		using BucketsType = DynArray<bucket_t>;
		BucketsType _buckets;

		size_t _size = 0;
		EqCmp _equality_comparator;
		Hasher _hasher;

		PBOS_FORCEINLINE int _check_capacity() {
			size_t capacity = _buckets.size() << 1;

			if (capacity < _size)
				return 1;
			if (auto size = (capacity >> 1); size > _size) {
				if (size <= 1)
					return 0;
				return -1;
			}
			return 0;
		}

		[[nodiscard]] PBOS_FORCEINLINE static bool _resize_buckets(size_t new_size, BucketsType &old_buckets, BucketsType &new_buckets) {
			{
				if (!new_buckets.resize_uninit(new_size)) {
					return false;
				}
				for (size_t i = 0; i < new_buckets.size(); ++i) {
					construct_at<bucket_t>(&new_buckets.at(i), new_buckets.allocator());
				}
			}

			const size_t n_old_buckets = old_buckets.size();
			ScopeGuard restore_guard([new_size, &old_buckets, &new_buckets, n_old_buckets]() noexcept {
				for (size_t i = 0; i < new_size; ++i) {
					bucket_t &bucket = new_buckets.at(i);

					for (typename bucket_t::NodeHandle j = bucket.first_node(); j; j = j->next) {
						size_t index = ((size_t)j->data.hash_code) % n_old_buckets;

						bucket.detach(j);
						old_buckets.at(index).push_front(j);
					}
				}
				new_buckets.clear_and_shrink();
			});

			for (size_t i = 0; i < n_old_buckets; ++i) {
				bucket_t &bucket = old_buckets.at(i);

				for (typename bucket_t::NodeHandle j = bucket.first_node(); j;) {
					typename bucket_t::NodeHandle next = j->next;
					size_t index = ((size_t)j->data.hash_code) % new_size;

					bucket.detach(j);
					new_buckets.at(index).push_front(j);
					j = next;
				}
			}

			restore_guard.release();

			return true;
		}

		[[nodiscard]] PBOS_FORCEINLINE bucket_node_handle_query_result_t _get_bucket_slot(const bucket_t &bucket, const T &data) const {
			for (auto i = bucket.first_node(); i; i = i->next) {
				if constexpr (Fallible) {
					if (auto result = _equality_comparator(i->data.data, data); result.hasValue()) {
						return i;
					} else {
						return NULL_OPTION;
					}
				} else {
					if (_equality_comparator(i->data.data, data)) {
						return i;
					}
				}
			}

			return bucket_t::null_node_handle();
		}

		[[nodiscard]] PBOS_FORCEINLINE bool _check_and_resize_buckets() {
			size_t size = _buckets.size();

			switch (_check_capacity()) {
				case 1: {
					BucketsType new_buckets(_buckets.allocator());
					if (!_resize_buckets(size ? size << 1 : 1, _buckets, new_buckets)) {
						return false;
					}
					_buckets = std::move(new_buckets);
					break;
				}
				case 0:
					break;
				case -1: {
					BucketsType new_buckets(_buckets.allocator());
					if (!_resize_buckets(size >> 1, _buckets, new_buckets)) {
						return false;
					}
					_buckets = std::move(new_buckets);
					break;
				}
			}

			return true;
		}

		PBOS_FORCEINLINE bool _is_buckets_initialized() {
			return _buckets.size();
		}

		/// @brief Insert a new element.
		/// @param buckets Buckets to be operated.
		/// @param data Element to insert.
		/// @return true for succeeded, false if failed.
		[[nodiscard]] PBOS_FORCEINLINE bool _insert(T &&data, bool force_resize_buckets) {
			if (!_buckets.size()) {
				if (!_buckets.resize_uninit(1)) {
					return false;
				}

				construct_at<bucket_t>(&_buckets.at(0), _buckets.allocator());
			}

			T tmp_data = std::move(data);

			hash_code_t hash_code;
			if constexpr (Fallible) {
				if (auto result = _hasher(tmp_data); result.hasValue()) {
					hash_code = result.value();
				} else
					return false;
			} else {
				hash_code = _hasher(tmp_data);
			}
			size_t index = ((size_t)hash_code) % _buckets.size();
			bucket_t &bucket = _buckets.at(index);

			for (auto &i : bucket) {
				if (_equality_comparator(i.data, tmp_data)) {
					move_assign_or_move_construct<T>(i.data, std::move(tmp_data));
					goto inserted;
				}
			}
			if (!bucket.push_front(element_t(std::move(tmp_data), hash_code)))
				return false;
		inserted:

			if (!_check_and_resize_buckets()) {
				if (force_resize_buckets) {
					bucket.pop_front();
					return false;
				}
			}

			++_size;
			return true;
		}

		[[nodiscard]] PBOS_FORCEINLINE RemoveResultType _remove(const T &data, bool force_resize_buckets) {
			if (!_buckets.size()) {
				if constexpr (Fallible) {
					return true;
				} else {
					return;
				}
			}

			hash_code_t hash_code;
			if constexpr (Fallible) {
				if (auto result = _hasher(data); result.hasValue()) {
					hash_code = result.value();
				} else
					return false;
			} else {
				hash_code = _hasher(data);
			}
			size_t index = ((size_t)hash_code) % _buckets.size();
			bucket_t &bucket = _buckets.at(index);
			RcObjectPtr<Allocator> alloc = bucket.allocator();

			typename bucket_t::NodeHandle node;

			if constexpr (Fallible) {
				bucket_node_handle_query_result_t maybe_node = _get_bucket_slot(bucket, data);
				if (!maybe_node.hasValue()) {
					return false;
				}

				node = maybe_node.value();
			} else {
				node = _get_bucket_slot(bucket, data);
			}

			if (node) {
				typename bucket_t::NodeHandle next_node = bucket_t::next(node, 1);

				bucket.detach(node);
				bucket.delete_node(node);

				--_size;
			}

			if constexpr (Fallible) {
				return true;
			}
		}

		[[nodiscard]] PBOS_FORCEINLINE bucket_node_handle_query_result_t _get(const T &data, size_t &index) const {
			if (!_buckets.size()) {
				return bucket_t::null_node_handle();
			}

			hash_code_t hash_code;
			if constexpr (Fallible) {
				if (auto result = _hasher(data); result.hasValue()) {
					hash_code = result.value();
				} else
					return NULL_OPTION;
			} else {
				hash_code = _hasher(data);
			}
			size_t i = ((size_t)hash_code) % _buckets.size();
			const bucket_t &bucket = _buckets.at(i);

			return _get_bucket_slot(bucket, data);
		}

	public:
		PBOS_FORCEINLINE HashSetImpl(Allocator *allocator) : _buckets(allocator) {
		}

		PBOS_FORCEINLINE HashSetImpl(ThisType &&other)
			: _buckets(std::move(other._buckets)),
			  _size(other._size),
			  _equality_comparator(std::move(other._equality_comparator)),
			  _hasher(std::move(other._hasher)) {
			other._size = 0;
		}

		PBOS_FORCEINLINE ThisType &operator=(ThisType &&other) noexcept {
			clear_and_shrink();

			_buckets = std::move(other._buckets);
			_size = other._size;
			_equality_comparator = std::move(other._equality_comparator);
			_hasher = std::move(other._hasher);

			other._size = 0;

			return *this;
		}

		[[nodiscard]] PBOS_FORCEINLINE bool insert_without_resize_buckets(T &&data) {
			return _insert(std::move(data), false);
		}

		[[nodiscard]] PBOS_FORCEINLINE bool insert(T &&data) {
			return _insert(std::move(data), true);
		}

		[[nodiscard]] PBOS_FORCEINLINE RemoveResultType remove(const T &data) {
			if constexpr (Fallible) {
				return _remove(data, false).hasValue();
			} else {
				_remove(data, false);
			}
		}

		[[nodiscard]] PBOS_FORCEINLINE bucket_node_handle_query_result_t get(const T &data) {
			size_t index;
			return _get(data, index);
		}

		[[nodiscard]] PBOS_FORCEINLINE ContainsResultType contains(const T &data) const {
			size_t index;
			if constexpr (Fallible) {
				auto maybe_handle = _get(data, index);

				if (!maybe_handle.hasValue())
					return NULL_OPTION;

				return maybe_handle.value();
			} else {
				return _get(data, index);
			}
		}

		PBOS_FORCEINLINE void clear() {
			_buckets.clear();
		}

		PBOS_FORCEINLINE void clear_and_shrink() {
			_buckets.clear_and_shrink();
		}

		PBOS_FORCEINLINE Allocator *allocator() const {
			return _buckets.allocator();
		}

		PBOS_FORCEINLINE void replace_allocator(Allocator *rhs) noexcept {
			for (auto &i : _buckets) {
				i.replaceAllocator(rhs);
			}
			_buckets.replaceAllocator(rhs);
		}

		struct Iterator {
			size_t idx_cur_bucket;
			typename bucket_t::NodeHandle bucket_node_handle;
			ThisType *hash_set;
			IteratorDirection direction;

			PBOS_FORCEINLINE Iterator(
				ThisType *hash_set,
				size_t idx_cur_bucket,
				typename bucket_t::NodeHandle bucket_node_handle,
				IteratorDirection direction)
				: idx_cur_bucket(idx_cur_bucket),
				  bucket_node_handle(bucket_node_handle),
				  hash_set(hash_set),
				  direction(direction) {}

			Iterator(const Iterator &it) = default;
			PBOS_FORCEINLINE Iterator(Iterator &&it) {
				idx_cur_bucket = it.idx_cur_bucket;
				bucket_node_handle = it.bucket_node_handle;
				hash_set = it.hash_set;
				direction = it.direction;

				it.idx_cur_bucket = SIZE_MAX;
				it.bucket_node_handle = bucket_t::null_node_handle();
				it.hash_set = nullptr;
				it.direction = IteratorDirection::Invalid;
			}
			PBOS_FORCEINLINE Iterator &operator=(const Iterator &rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible Iterator direction");
				idx_cur_bucket = rhs.idx_cur_bucket;
				bucket_node_handle = rhs.bucket_node_handle;
				hash_set = rhs.hash_set;
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
				if (idx_cur_bucket == SIZE_MAX)
					km_panic("Increasing the end Iterator");

				if (direction == IteratorDirection::Forward) {
					typename bucket_t::NodeHandle next_node = bucket_t::next(bucket_node_handle, 1);
					if (!next_node) {
						while ((!next_node) && (idx_cur_bucket != SIZE_MAX)) {
							if (++idx_cur_bucket >= hash_set->_buckets.size()) {
								idx_cur_bucket = SIZE_MAX;
								next_node = nullptr;
							} else {
								next_node = hash_set->_buckets.at(idx_cur_bucket).first_node();
							}
						}
					}
					bucket_node_handle = next_node;
				} else {
					typename bucket_t::NodeHandle next_node = bucket_t::prev(bucket_node_handle, 1);
					if (!next_node) {
						while ((!next_node) && (idx_cur_bucket != SIZE_MAX)) {
							if (!idx_cur_bucket) {
								idx_cur_bucket = SIZE_MAX;
								next_node = nullptr;
							} else {
								--idx_cur_bucket;
								next_node = hash_set->_buckets.at(idx_cur_bucket).last_node();
							}
						}
					}
					bucket_node_handle = next_node;
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
					if (idx_cur_bucket == SIZE_MAX) {
						idx_cur_bucket = hash_set->_buckets.size();
						bucket_node_handle = hash_set->_buckets.at(idx_cur_bucket).last_node();
					} else {
						typename bucket_t::NodeHandle next_node = bucket_t::prev(bucket_node_handle, 1);
						if (!next_node) {
							while (!next_node) {
								if (!idx_cur_bucket) {
									km_panic("Decreasing the beginning Iterator");
								} else {
									--idx_cur_bucket;
									next_node = hash_set->_buckets.at(idx_cur_bucket).last_node();
								}
							}
						}
						bucket_node_handle = next_node;
					}
				} else {
					if (idx_cur_bucket == SIZE_MAX) {
						idx_cur_bucket = 0;
						bucket_node_handle = hash_set->_buckets.at(0).first_node();
					} else {
						typename bucket_t::NodeHandle next_node = bucket_t::next(bucket_node_handle, 1);
						if (!next_node) {
							while (!next_node) {
								if (++idx_cur_bucket >= hash_set->_buckets.size()) {
									km_panic("Decreasing the beginning Iterator");
								} else {
									next_node = hash_set->_buckets.at(idx_cur_bucket).first_node();
								}
							}
						}
						bucket_node_handle = next_node;
					}
				}

				return *this;
			}

			PBOS_FORCEINLINE Iterator operator--(int) {
				Iterator it = *this;
				--(*this);
				return it;
			}

			PBOS_FORCEINLINE bool operator==(const Iterator &it) const {
				if (hash_set != it.hash_set)
					km_panic("Cannot compare iterators from different containers");
				return bucket_node_handle == it.bucket_node_handle;
			}

			PBOS_FORCEINLINE bool operator==(const Iterator &&rhs) const {
				const Iterator it = rhs;
				return *this == it;
			}

			PBOS_FORCEINLINE bool operator!=(const Iterator &it) const {
				if (hash_set != it.hash_set)
					km_panic("Cannot compare iterators from different containers");
				return bucket_node_handle != it.bucket_node_handle;
			}

			PBOS_FORCEINLINE bool operator!=(Iterator &&rhs) const {
				Iterator it = rhs;
				return *this != it;
			}

			PBOS_FORCEINLINE T &operator*() {
				if (!bucket_node_handle)
					km_panic("Deferencing the end Iterator");
				return bucket_node_handle->data.data;
			}

			PBOS_FORCEINLINE T &operator*() const {
				if (!bucket_node_handle)
					km_panic("Deferencing the end Iterator");
				return bucket_node_handle->data.data;
			}

			PBOS_FORCEINLINE T *operator->() {
				if (!bucket_node_handle)
					km_panic("Deferencing the end Iterator");
				return &bucket_node_handle->data.data;
			}

			PBOS_FORCEINLINE T *operator->() const {
				if (!bucket_node_handle)
					km_panic("Deferencing the end Iterator");
				return &bucket_node_handle->data.data;
			}
		};

		PBOS_FORCEINLINE Iterator begin() {
			for (size_t i = 0; i < _buckets.size(); ++i) {
				auto &cur_bucket = _buckets.at(i);
				typename bucket_t::NodeHandle node = cur_bucket.first_node();
				if (node) {
					return Iterator(this, i, node, IteratorDirection::Forward);
				}
			}
			return end();
		}
		PBOS_FORCEINLINE Iterator end() {
			return Iterator(this, SIZE_MAX, nullptr, IteratorDirection::Forward);
		}
		PBOS_FORCEINLINE Iterator begin_reversed() {
			for (size_t i = _buckets.size(); i; --i) {
				auto &cur_bucket = _buckets.at(i);
				typename bucket_t::NodeHandle node = cur_bucket.last_node();
				if (node) {
					return Iterator(this, i, node, IteratorDirection::Reversed);
				}
			}

			auto &cur_bucket = _buckets.at(0);
			typename bucket_t::NodeHandle node = cur_bucket.last_node();
			if (node) {
				return Iterator(this, 0, node, IteratorDirection::Reversed);
			}
			return end_reversed();
		}

		PBOS_FORCEINLINE Iterator end_reversed() {
			return Iterator(this, SIZE_MAX, nullptr, IteratorDirection::Reversed);
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
				return _iterator == rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(ConstIterator &&rhs) const {
				return _iterator == rhs._iterator;
			}

			PBOS_FORCEINLINE T &operator*() {
				return *_iterator;
			}

			PBOS_FORCEINLINE const T &operator*() const {
				return *_iterator;
			}

			PBOS_FORCEINLINE T *operator->() {
				return &*_iterator;
			}

			PBOS_FORCEINLINE const T *operator->() const {
				return &*_iterator;
			}

			PBOS_FORCEINLINE ConstIterator &operator++() {
				++_iterator;
				return *this;
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

		PBOS_FORCEINLINE Iterator find(const T &value) {
			size_t index;
			if constexpr (Fallible) {
				bucket_node_handle_query_result_t node = _get(value, index);
				if (!node.hasValue())
					return end();
				if (typename bucket_t::NodeHandle handle = node.value(); handle)
					return Iterator(this, index, handle, IteratorDirection::Forward);
				return end();
			} else {
				typename bucket_t::NodeHandle node = _get(value, index);
				if (!node)
					return end();
				return Iterator(this, index, node, IteratorDirection::Forward);
			}
		}

		PBOS_FORCEINLINE ConstIterator find(const T &value) const {
			return ConstIterator(const_cast<ThisType *>(this)->find(value));
		}

		PBOS_FORCEINLINE element_query_result_t at(const T &value) {
			size_t index;
			typename bucket_t::NodeHandle node;
			if constexpr (Fallible) {
				auto maybe_node = _get(value, index);

				if (!maybe_node.hasValue())
					return NULL_OPTION;

				node = maybe_node.value();
			} else {
				node = _get(value, index);
			}
			if (!node)
				km_panic("No such element");
			return node->data.data;
		}

		PBOS_FORCEINLINE const_element_query_result_t at(const T &value) const {
			size_t index;
			typename bucket_t::NodeHandle node;
			if constexpr (Fallible) {
				auto maybe_node = _get(value, index);

				if (!maybe_node.hasValue())
					return NULL_OPTION;

				node = maybe_node.value();
			} else {
				node = _get(value, index);
			}
			if (!node)
				km_panic("No such element");
			return node->data.data;
		}

		PBOS_FORCEINLINE size_t size() const {
			return _size;
		}

		PBOS_FORCEINLINE bool shrink_buckets() {
			return _buckets.shrink_to_fit();
		}
	};

	template <typename T, typename EqCmp = std::equal_to<T>, typename Hasher = kfxx::Hash<T>>
	using HashSet = HashSetImpl<T, EqCmp, Hasher, false>;
	template <typename T, typename EqCmp = FallibleEq<T>, typename Hasher = FallibleHash<T>>
	using FallibleHashSet = HashSetImpl<T, EqCmp, Hasher, true>;
}

#endif
