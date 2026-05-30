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
		struct hash_code_result_type_extractor {
			using type = T;
		};

		template <typename T>
		struct hash_code_result_type_extractor<T, std::void_t<decltype(std::declval<T>().value())>> {
			using type = typename T::value_type;
		};
	}

	template <
		typename T,
		typename EqCmp,
		typename Hasher,
		bool Fallible>
	// PBOS_REQUIRES_CONCEPT(std::invocable<EqCmp, const T &, const T &>)
	class hashset_impl {
	public:
		static_assert(std::is_move_constructible_v<T>, "The element must be move-constructible");
		using HashResult = decltype(std::declval<Hasher>()(std::declval<T>()));
		using HashCode = typename details::hash_code_result_type_extractor<HashResult>::type;

		struct element_t {
			T data;
			HashCode hash_code;

			PBOS_FORCEINLINE element_t(T &&data, HashCode hash_code) : data(std::move(data)), hash_code(hash_code) {}

			element_t(element_t &&rhs) = default;
			element_t &operator=(element_t &&rhs) = default;
		};

		using Bucket = list_t<element_t>;

	public:
		using RemoveResultType = typename std::conditional_t<Fallible, bool, void>;
		using ElementQueryResult = typename std::conditional_t<Fallible, option_t<T &>, T &>;
		using BucketNodeHandleQueryResult = typename std::conditional_t<Fallible, option_t<typename Bucket::NodeHandle>, typename Bucket::NodeHandle>;
		using ConstElementQueryResult = typename std::conditional_t<Fallible, option_t<const T &>, const T &>;
		using ContainsResultType = typename std::conditional_t<Fallible, option_t<bool>, bool>;

	private:
		using ThisType = hashset_impl<T, EqCmp, Hasher, Fallible>;

		using BucketsType = dynarray_t<Bucket>;
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
					construct_at<Bucket>(&new_buckets.at(i), new_buckets.allocator());
				}
			}

			const size_t n_old_buckets = old_buckets.size();
			scope_guard restore_guard([new_size, &old_buckets, &new_buckets, n_old_buckets]() noexcept {
				for (size_t i = 0; i < new_size; ++i) {
					Bucket &bucket = new_buckets.at(i);

					for (typename Bucket::NodeHandle j = bucket.first_node(); j; j = j->next) {
						size_t index = ((size_t)j->data.hash_code) % n_old_buckets;

						bucket.detach(j);
						old_buckets.at(index).push_front(j);
					}
				}
				new_buckets.clear_and_shrink();
			});

			for (size_t i = 0; i < n_old_buckets; ++i) {
				Bucket &bucket = old_buckets.at(i);

				for (typename Bucket::NodeHandle j = bucket.first_node(); j;) {
					typename Bucket::NodeHandle next = j->next;
					size_t index = ((size_t)j->data.hash_code) % new_size;

					bucket.detach(j);
					new_buckets.at(index).push_front(j);
					j = next;
				}
			}

			restore_guard.release();

			return true;
		}

		[[nodiscard]] PBOS_FORCEINLINE BucketNodeHandleQueryResult _get_bucket_slot(const Bucket &bucket, const T &data) const {
			for (auto i = bucket.first_node(); i; i = i->next) {
				if constexpr (Fallible) {
					if (auto result = _equality_comparator(i->data.data, data); result.hasValue()) {
						return i;
					} else {
						return nullopt;
					}
				} else {
					if (_equality_comparator(i->data.data, data)) {
						return i;
					}
				}
			}

			return Bucket::null_node_handle();
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

				construct_at<Bucket>(&_buckets.at(0), _buckets.allocator());
			}

			T tmp_data = std::move(data);

			HashCode hash_code;
			if constexpr (Fallible) {
				if (auto result = _hasher(tmp_data); result.hasValue()) {
					hash_code = result.value();
				} else
					return false;
			} else {
				hash_code = _hasher(tmp_data);
			}
			size_t index = ((size_t)hash_code) % _buckets.size();
			Bucket &bucket = _buckets.at(index);

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

			HashCode hash_code;
			if constexpr (Fallible) {
				if (auto result = _hasher(data); result.hasValue()) {
					hash_code = result.value();
				} else
					return false;
			} else {
				hash_code = _hasher(data);
			}
			size_t index = ((size_t)hash_code) % _buckets.size();
			Bucket &bucket = _buckets.at(index);
			rc_object_ptr<allocator_t> alloc = bucket.allocator();

			typename Bucket::NodeHandle node;

			if constexpr (Fallible) {
				BucketNodeHandleQueryResult maybe_node = _get_bucket_slot(bucket, data);
				if (!maybe_node.hasValue()) {
					return false;
				}

				node = maybe_node.value();
			} else {
				node = _get_bucket_slot(bucket, data);
			}

			if (node) {
				typename Bucket::NodeHandle next_node = Bucket::next(node, 1);

				bucket.detach(node);
				bucket.delete_node(node);

				--_size;
			}

			if constexpr (Fallible) {
				return true;
			}
		}

		[[nodiscard]] PBOS_FORCEINLINE BucketNodeHandleQueryResult _get(const T &data, size_t &index) const {
			if (!_buckets.size()) {
				return Bucket::null_node_handle();
			}

			HashCode hash_code;
			if constexpr (Fallible) {
				if (auto result = _hasher(data); result.hasValue()) {
					hash_code = result.value();
				} else
					return nullopt;
			} else {
				hash_code = _hasher(data);
			}
			size_t i = ((size_t)hash_code) % _buckets.size();
			const Bucket &bucket = _buckets.at(i);

			return _get_bucket_slot(bucket, data);
		}

	public:
		PBOS_FORCEINLINE hashset_impl(allocator_t *allocator) : _buckets(allocator) {
		}

		PBOS_FORCEINLINE hashset_impl(ThisType &&other)
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

		[[nodiscard]] PBOS_FORCEINLINE BucketNodeHandleQueryResult get(const T &data) {
			size_t index;
			return _get(data, index);
		}

		[[nodiscard]] PBOS_FORCEINLINE ContainsResultType contains(const T &data) const {
			size_t index;
			if constexpr (Fallible) {
				auto maybe_handle = _get(data, index);

				if (!maybe_handle.hasValue())
					return nullopt;

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

		PBOS_FORCEINLINE allocator_t *allocator() const {
			return _buckets.allocator();
		}

		PBOS_FORCEINLINE void replace_allocator(allocator_t *rhs) noexcept {
			for (auto &i : _buckets) {
				i.replaceAlloc(rhs);
			}
			_buckets.replaceAlloc(rhs);
		}

		struct iterator {
			size_t idx_cur_bucket;
			typename Bucket::NodeHandle bucket_node_handle;
			ThisType *hash_set;
			iterator_direction direction;

			PBOS_FORCEINLINE iterator(
				ThisType *hash_set,
				size_t idx_cur_bucket,
				typename Bucket::NodeHandle bucket_node_handle,
				iterator_direction direction)
				: idx_cur_bucket(idx_cur_bucket),
				  bucket_node_handle(bucket_node_handle),
				  hash_set(hash_set),
				  direction(direction) {}

			iterator(const iterator &it) = default;
			PBOS_FORCEINLINE iterator(iterator &&it) {
				idx_cur_bucket = it.idx_cur_bucket;
				bucket_node_handle = it.bucket_node_handle;
				hash_set = it.hash_set;
				direction = it.direction;

				it.idx_cur_bucket = SIZE_MAX;
				it.bucket_node_handle = Bucket::null_node_handle();
				it.hash_set = nullptr;
				it.direction = iterator_direction::Invalid;
			}
			PBOS_FORCEINLINE iterator &operator=(const iterator &rhs) noexcept {
				if (direction != rhs.direction)
					km_panic("Incompatible iterator direction");
				idx_cur_bucket = rhs.idx_cur_bucket;
				bucket_node_handle = rhs.bucket_node_handle;
				hash_set = rhs.hash_set;
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
				if (idx_cur_bucket == SIZE_MAX)
					km_panic("Increasing the end iterator");

				if (direction == iterator_direction::Forward) {
					typename Bucket::NodeHandle next_node = Bucket::next(bucket_node_handle, 1);
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
					typename Bucket::NodeHandle next_node = Bucket::prev(bucket_node_handle, 1);
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

			PBOS_FORCEINLINE iterator operator++(int) {
				iterator it = *this;
				++(*this);
				return it;
			}

			PBOS_FORCEINLINE iterator &operator--() {
				if (direction == iterator_direction::Forward) {
					if (idx_cur_bucket == SIZE_MAX) {
						idx_cur_bucket = hash_set->_buckets.size();
						bucket_node_handle = hash_set->_buckets.at(idx_cur_bucket).last_node();
					} else {
						typename Bucket::NodeHandle next_node = Bucket::prev(bucket_node_handle, 1);
						if (!next_node) {
							while (!next_node) {
								if (!idx_cur_bucket) {
									km_panic("Decreasing the beginning iterator");
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
						typename Bucket::NodeHandle next_node = Bucket::next(bucket_node_handle, 1);
						if (!next_node) {
							while (!next_node) {
								if (++idx_cur_bucket >= hash_set->_buckets.size()) {
									km_panic("Decreasing the beginning iterator");
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

			PBOS_FORCEINLINE iterator operator--(int) {
				iterator it = *this;
				--(*this);
				return it;
			}

			PBOS_FORCEINLINE bool operator==(const iterator &it) const {
				if (hash_set != it.hash_set)
					km_panic("Cannot compare iterators from different containers");
				return bucket_node_handle == it.bucket_node_handle;
			}

			PBOS_FORCEINLINE bool operator==(const iterator &&rhs) const {
				const iterator it = rhs;
				return *this == it;
			}

			PBOS_FORCEINLINE bool operator!=(const iterator &it) const {
				if (hash_set != it.hash_set)
					km_panic("Cannot compare iterators from different containers");
				return bucket_node_handle != it.bucket_node_handle;
			}

			PBOS_FORCEINLINE bool operator!=(iterator &&rhs) const {
				iterator it = rhs;
				return *this != it;
			}

			PBOS_FORCEINLINE T &operator*() {
				if (!bucket_node_handle)
					km_panic("Deferencing the end iterator");
				return bucket_node_handle->data.data;
			}

			PBOS_FORCEINLINE T &operator*() const {
				if (!bucket_node_handle)
					km_panic("Deferencing the end iterator");
				return bucket_node_handle->data.data;
			}

			PBOS_FORCEINLINE T *operator->() {
				if (!bucket_node_handle)
					km_panic("Deferencing the end iterator");
				return &bucket_node_handle->data.data;
			}

			PBOS_FORCEINLINE T *operator->() const {
				if (!bucket_node_handle)
					km_panic("Deferencing the end iterator");
				return &bucket_node_handle->data.data;
			}
		};

		PBOS_FORCEINLINE iterator begin() {
			for (size_t i = 0; i < _buckets.size(); ++i) {
				auto &cur_bucket = _buckets.at(i);
				typename Bucket::NodeHandle node = cur_bucket.first_node();
				if (node) {
					return iterator(this, i, node, iterator_direction::Forward);
				}
			}
			return end();
		}
		PBOS_FORCEINLINE iterator end() {
			return iterator(this, SIZE_MAX, nullptr, iterator_direction::Forward);
		}
		PBOS_FORCEINLINE iterator begin_reversed() {
			for (size_t i = _buckets.size(); i; --i) {
				auto &cur_bucket = _buckets.at(i);
				typename Bucket::NodeHandle node = cur_bucket.last_node();
				if (node) {
					return iterator(this, i, node, iterator_direction::Reversed);
				}
			}

			auto &cur_bucket = _buckets.at(0);
			typename Bucket::NodeHandle node = cur_bucket.last_node();
			if (node) {
				return iterator(this, 0, node, iterator_direction::Reversed);
			}
			return end_reversed();
		}

		PBOS_FORCEINLINE iterator end_reversed() {
			return iterator(this, SIZE_MAX, nullptr, iterator_direction::Reversed);
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
				return _iterator == rhs._iterator;
			}

			PBOS_FORCEINLINE bool operator!=(const_iterator &&rhs) const {
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

			PBOS_FORCEINLINE const_iterator &operator++() {
				++_iterator;
				return *this;
			}
		};

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

		PBOS_FORCEINLINE iterator find(const T &value) {
			size_t index;
			if constexpr (Fallible) {
				BucketNodeHandleQueryResult node = _get(value, index);
				if (!node.hasValue())
					return end();
				if (typename Bucket::NodeHandle handle = node.value(); handle)
					return iterator(this, index, handle, iterator_direction::Forward);
				return end();
			} else {
				typename Bucket::NodeHandle node = _get(value, index);
				if (!node)
					return end();
				return iterator(this, index, node, iterator_direction::Forward);
			}
		}

		PBOS_FORCEINLINE const_iterator find(const T &value) const {
			return const_iterator(const_cast<ThisType *>(this)->find(value));
		}

		PBOS_FORCEINLINE ElementQueryResult at(const T &value) {
			size_t index;
			typename Bucket::NodeHandle node;
			if constexpr (Fallible) {
				auto maybe_node = _get(value, index);

				if (!maybe_node.hasValue())
					return nullopt;

				node = maybe_node.value();
			} else {
				node = _get(value, index);
			}
			if (!node)
				km_panic("No such element");
			return node->data.data;
		}

		PBOS_FORCEINLINE ConstElementQueryResult at(const T &value) const {
			size_t index;
			typename Bucket::NodeHandle node;
			if constexpr (Fallible) {
				auto maybe_node = _get(value, index);

				if (!maybe_node.hasValue())
					return nullopt;

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

	template <typename T, typename EqCmp = std::equal_to<T>, typename Hasher = kfxx::hash<T>>
	using hashset_t = hashset_impl<T, EqCmp, Hasher, false>;
	template <typename T, typename EqCmp = fallible_equal_to<T>, typename Hasher = fallible_hash<T>>
	using fallible_hashset_t = hashset_impl<T, EqCmp, Hasher, true>;
}

#endif
