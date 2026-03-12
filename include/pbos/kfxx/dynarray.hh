#ifndef _PEFF_UTILS_DYNARRAY_H_
#define _PEFF_UTILS_DYNARRAY_H_

#include <pbos/km/assert.h>
#include <string.h>
#include <initializer_list>
#include "allocator.hh"
#include "basedefs.hh"
#include "rcobj.hh"
#include "scope_guard.hh"
#include "utils.hh"

namespace kfxx {
	/// @brief The dynamic array type.
	/// @tparam T Type of the elements.
	template <typename T>
	class dynarray_t {
	public:
		using iterator = T *;
		using const_iterator = const T *;

		size_t _length = 0;
		size_t _capacity = 0;
		rc_object_ptr_t<allocator_t> _allocator;
		T *_data;

		PBOS_FORCEINLINE static size_t _get_grown_capacity(size_t length, size_t old_capacity) {
			if (!old_capacity)
				return length;

			size_t new_capacity = old_capacity + (old_capacity >> 1);

			if (new_capacity < length)
				return length;

			return new_capacity;
		}

		PBOS_FORCEINLINE static size_t _get_shrinked_capacity(size_t length, size_t old_capacity) {
			if (!old_capacity)
				return length;

			size_t new_capacity = old_capacity >> 1;

			if (new_capacity > length)
				return length;

			return new_capacity;
		}

		PBOS_FORCEINLINE static int _check_capacity(size_t length, size_t capacity) {
			if (length > capacity)
				return 1;
			if (capacity < (length >> 1))
				return -1;
			return 0;
		}

		PBOS_FORCEINLINE void _move_data(T *new_data, T *old_data, size_t length) noexcept {
			if constexpr (std::is_trivially_move_assignable_v<T>) {
				memmove(new_data, old_data, sizeof(T) * length);
			} else {
				if (new_data + length < old_data) {
					for (size_t i = 0; i < length; ++i) {
						move_assign_or_move_construct<T>(new_data[i], std::move(old_data[i]));
					}
				} else {
					for (size_t i = length; i > 0; --i) {
						move_assign_or_move_construct<T>(new_data[i - 1], std::move(old_data[i - 1]));
					}
				}
			}
		}

		PBOS_FORCEINLINE void _move_data_uninit(T *new_data, T *old_data, size_t length) noexcept {
			if constexpr (std::is_trivially_move_constructible_v<T>) {
				memmove(new_data, old_data, sizeof(T) * length);
			} else {
				if (new_data + length < old_data) {
					for (size_t i = 0; i < length; ++i) {
						construct_at<T>(&new_data[i], std::move(old_data[i]));
					}
				} else {
					for (size_t i = length; i > 0; --i) {
						construct_at<T>(&new_data[i - 1], std::move(old_data[i - 1]));
					}
				}
			}
		}

		PBOS_FORCEINLINE void _construct_data(T *new_data, size_t length) {
			if constexpr (!std::is_trivially_constructible_v<T>) {
				for (size_t i = 0; i < length; ++i) {
					construct_at<T>(&new_data[i]);
				}
			}
		}

		PBOS_FORCEINLINE void _destruct_data(T *original_data, size_t length) {
			if constexpr (!std::is_trivially_destructible_v<T>) {
				for (size_t i = 0; i < length; ++i) {
					std::destroy_at<T>(&original_data[i]);
				}
			}
		}

		template <bool construct>
		[[nodiscard]] PBOS_FORCEINLINE void _expand_to(
			T *new_data,
			size_t length) {
			kd_assert(length > _length);

			if constexpr (construct) {
				// Because construction of new objects may throw exceptions,
				// we choose to construct the new objects first.
				size_t idx_last_constructed_object;
				scope_guard scope_guard(
					[this, &idx_last_constructed_object, new_data]() noexcept {
						for (size_t i = _length;
							i < idx_last_constructed_object;
							++i) {
							std::destroy_at<T>(&new_data[i]);
						}
					});

				for (size_t i = _length;
					i < length;
					++i) {
					idx_last_constructed_object = i;
					construct_at<T>(&new_data[i]);
				}

				scope_guard.release();
			}

			if (new_data != _data) {
				if (_data)
					_move_data_uninit(new_data, _data, _length);
			}
		}

		PBOS_FORCEINLINE void _shrink(
			T *new_data,
			size_t length) noexcept {
			kd_assert(length < _length);

			for (size_t i = length; i < _length; ++i) {
				std::destroy_at<T>(&_data[i]);
			}

			if (new_data != _data) {
				if (_data)
					_move_data_uninit(new_data, _data, length);
			}
		}

		template <bool construct, bool force_resize_capacity>
		[[nodiscard]] PBOS_FORCEINLINE bool _resize(size_t length) {
			if (!length) {
				if constexpr (force_resize_capacity) {
					_clear();
				} else {
					_shrink(_data, 0);
				}
				return true;
			}

			int capacity_status = _check_capacity(length, _capacity);
			if (capacity_status > 0) {
				size_t new_capacity = _get_grown_capacity(length, _capacity);

				size_t new_capacity_total_size = new_capacity * sizeof(T);
				T *new_data;
				bool clear_old_data = true;

				if constexpr (std::is_trivially_move_assignable_v<T>) {
					if (_data) {
						if (!(new_data = (T *)_allocator->realloc(_data, sizeof(T) * _capacity, alignof(T), new_capacity_total_size, alignof(T))))
							return false;
						clear_old_data = false;
					} else {
						if (!(new_data = (T *)_allocator->alloc(new_capacity_total_size, alignof(T))))
							return false;
					}
				} else {
					if (_data) {
						if (!(new_data = (T *)_allocator->alloc(new_capacity_total_size, alignof(T))))
							return false;
						if constexpr (std::is_nothrow_constructible_v<T>) {
							_expandTo<construct>(new_data, length);
						} else {
							scope_guard sg(
								[this, new_capacity_total_size, new_data]() noexcept {
									_allocator->release(new_data, new_capacity_total_size, alignof(T));
								});
							_expandTo<construct>(new_data, length);
							sg.release();
						}
					} else {
						if constexpr (std::is_nothrow_constructible_v<T>) {
							if ((new_data = (T *)_allocator->realloc_in_place(
									 _data,
									 sizeof(T) * _capacity, alignof(T),
									 new_capacity_total_size, alignof(T)))) {
								_expandTo<construct>(new_data, length);
							} else {
								if (!(new_data = (T *)_allocator->alloc(new_capacity_total_size, alignof(T))))
									return false;
								scope_guard sg(
									[this, new_capacity_total_size, new_data]() noexcept {
										_allocator->release(new_data, new_capacity_total_size, alignof(T));
									});
								_expandTo<construct>(new_data, length);
								sg.release();
							}
						} else {
							if (!(new_data = (T *)_allocator->alloc(new_capacity_total_size, alignof(T))))
								return false;
							scope_guard sg(
								[this, new_capacity_total_size, new_data]() noexcept {
									_allocator->release(new_data, new_capacity_total_size, alignof(T));
								});
							_expandTo<construct>(new_data, length);
							sg.release();
						}
					}
				}

				if (clear_old_data)
					_clear();
				_capacity = new_capacity;
				_data = new_data;
			} else if (capacity_status < 0) {
				if constexpr (force_resize_capacity) {
					size_t new_capacity = _get_shrinked_capacity(length, _capacity);

					size_t new_capacity_total_size = new_capacity * sizeof(T);
					T *new_data;
					bool clear_old_data = true;

					if constexpr (std::is_trivially_move_assignable_v<T>) {
						if (_data) {
							if (!(new_data = (T *)_allocator->realloc(_data, sizeof(T) * _capacity, alignof(T), new_capacity_total_size, alignof(T))))
								return false;
							clear_old_data = false;
						} else {
							if (!(new_data = (T *)_allocator->alloc(new_capacity_total_size, alignof(T))))
								return false;
						}
					} else {
						if (!(new_data = (T *)_allocator->alloc(new_capacity_total_size, alignof(T))))
							return false;
					}

					if (!new_data) {
						return false;
					}

					if constexpr (std::is_trivially_move_assignable_v<T>) {
					} else {
						_shrink(new_data, length);
					}

					if (clear_old_data)
						_clear();
					_capacity = new_capacity;
					_data = new_data;
				} else {
					_shrink(_data, length);
				}
			} else {
				if (length > _length) {
					if constexpr (!std::is_trivially_constructible_v<T>) {
						if (!_expand_to<construct>(_data, length))
							return false;
					}
				} else if (length < _length) {
					if constexpr (!std::is_trivially_destructible_v<T>) {
						_shrink(_data, length);
					}
				}
			}

			_length = length;
			return true;
		}

		PBOS_FORCEINLINE void _clear() {
			if constexpr (!std::is_trivial_v<T>) {
				for (size_t i = 0; i < _length; ++i)
					std::destroy_at<T>(&_data[i]);
			}
			if (_capacity) {
				_allocator->release(_data, sizeof(T) * _capacity, alignof(T));
			}

			_data = nullptr;
			_length = 0;
			_capacity = 0;
		}

		[[nodiscard]] PBOS_FORCEINLINE bool erase_range_and_shrink(size_t idx_start, size_t idx_end) {
			kd_assert(idx_start < _length);
			kd_assert(idx_end <= _length);

			const size_t gap_length = idx_end - idx_start;
			const size_t post_gap_length = _length - idx_end;
			const size_t new_length = _length - gap_length;

			if (post_gap_length < gap_length) {
				_destruct_data(_data + idx_start + post_gap_length, (gap_length - post_gap_length));
			}
			_move_data(_data + idx_start, _data + idx_end, _length - idx_end);

			return _resize<false, true>(new_length);
		}

		[[nodiscard]] PBOS_FORCEINLINE bool erase_range(size_t idx_start, size_t idx_end) {
			kd_assert(idx_start < _length);
			kd_assert(idx_end <= _length);

			const size_t gap_length = idx_end - idx_start;
			const size_t post_gap_length = _length - idx_end;
			const size_t new_length = _length - gap_length;

			if (post_gap_length < gap_length) {
				_destruct_data(_data + idx_start + post_gap_length, (gap_length - post_gap_length));
			}
			_move_data(_data + idx_start, _data + idx_end, _length - idx_end);

			_length = new_length;

			return true;
		}

		/// @brief Reserve an area in front of specified index.
		/// @param index Index to be reserved.
		/// @param length Length of space to reserve.
		/// @param construct Determines if to construct objects.
		/// @return Pointer to the reserved area.
		template <bool construct>
		[[nodiscard]] PBOS_FORCEINLINE T *_insert_range(
			size_t index,
			size_t length) {
			const size_t
				old_length = _length,
				new_length = _length + length;

			if (!_resize<construct, false>(new_length))
				return nullptr;

			T *gap_start = &_data[index];

			if (std::is_trivially_move_assignable_v<T>) {
				if (index < old_length) {
					memmove(&_data[index + length], gap_start, sizeof(T) * (old_length - index));
				}
			} else {
				if (index < old_length) {
					_move_data_uninit(
						&_data[index + length],
						gap_start,
						old_length - index);
				}

				if constexpr (construct) {
					_construct_data(gap_start, length);
				}
			}

			return gap_start;
		}

		using this_t = dynarray_t<T>;

	public:
		PBOS_FORCEINLINE dynarray_t(allocator_t *allocator_t) : _allocator(allocator_t), _data(nullptr) {
		}
		PBOS_FORCEINLINE dynarray_t(this_t &&rhs) noexcept : _allocator(std::move(rhs._allocator)), _data(std::move(rhs._data)), _length(rhs._length), _capacity(rhs._capacity) {
			rhs._data = nullptr;
			rhs._length = 0;
			rhs._capacity = 0;
		}
		PBOS_FORCEINLINE ~dynarray_t() {
			_clear();
		}

		PBOS_FORCEINLINE this_t &operator=(this_t &&rhs) noexcept {
			verify_allocator(_allocator.get(), rhs._allocator.get());
			_clear();

			_allocator = rhs._allocator;
			_data = rhs._data;
			_length = rhs._length;
			_capacity = rhs._capacity;

			rhs._data = nullptr;
			rhs._length = 0;
			rhs._capacity = 0;

			return *this;
		}

		PBOS_FORCEINLINE size_t size() {
			return _length;
		}

		[[nodiscard]] PBOS_FORCEINLINE bool resize_and_shrink(size_t length) {
			if (length == _length)
				return true;
			return _resize<true, true>(length);
		}

		[[nodiscard]] PBOS_FORCEINLINE bool resize(size_t length) {
			if (length == _length)
				return true;
			return _resize<true, false>(length);
		}

		[[nodiscard]] PBOS_FORCEINLINE bool resize_uninit_and_shrink(size_t length) {
			if (length == _length)
				return true;
			return _resize<false, true>(length);
		}

		[[nodiscard]] PBOS_FORCEINLINE bool resize_uninit(size_t length) {
			if (length == _length)
				return true;
			return _resize<false, false>(length);
		}

		[[nodiscard]] PBOS_FORCEINLINE bool shrink_to_fit() {
			return _resize<false, true>(_length);
		}

		[[nodiscard]] PBOS_FORCEINLINE bool build(const this_t &rhs) {
			clear();

			if (!resize_uninit(rhs.size())) {
				return false;
			}

			size_t i = 0;

			scope_guard destruct_guard([this, &i]() noexcept {
				if constexpr (!std::is_trivially_destructible_v<T>) {
					for (size_t j = 0; j < i; ++j) {
						std::destroy_at<T>(&_data[j]);
					}
				}
			});

			while (i < _length) {
				construct_at<T>(&_data[i], rhs._data[i]);
				++i;
			}

			destruct_guard.release();

			return true;
		}

		[[nodiscard]] PBOS_FORCEINLINE bool build(const std::initializer_list<T> &rhs) {
			clear();

			if (!resize_uninit(rhs.size())) {
				return false;
			}

			size_t i = 0;

			scope_guard destruct_guard([this, &i]() noexcept {
				if constexpr (!std::is_trivially_destructible_v<T>) {
					for (size_t j = 0; j < i; ++j) {
						std::destroy_at<T>(&_data[j]);
					}
				}
			});

			for (const auto &i : rhs) {
				construct_at<T>(&_data[i], i);
				++i;
			}

			destruct_guard.release();

			return true;
		}

		[[nodiscard]] PBOS_FORCEINLINE bool buildAndShrink(const this_t &rhs) {
			if (!build(rhs))
				return false;
			return shrink_to_fit();
		}

		[[nodiscard]] PBOS_FORCEINLINE bool buildAndShrink(const std::initializer_list<T> &rhs) {
			if (!build(rhs))
				return false;
			return shrink_to_fit();
		}

		PBOS_FORCEINLINE void clear() {
			bool result = resize_uninit(0);
			kd_assert(result);
		}

		PBOS_FORCEINLINE void clear_and_shrink() {
			_clear();
		}

		PBOS_FORCEINLINE T &at(size_t index) {
			kd_assert(index < _length);
			return _data[index];
		}

		PBOS_FORCEINLINE const T &at(size_t index) const {
			kd_assert(index < _length);
			return _data[index];
		}

		PBOS_FORCEINLINE size_t size() const {
			return _length;
		}

		[[nodiscard]] PBOS_FORCEINLINE bool insert_range_initialized(size_t index, size_t length) {
			if (!_insert_range<true>(index, length))
				return false;
			return true;
		}

		[[nodiscard]] PBOS_FORCEINLINE bool insert_range_uninit(size_t index, size_t length) {
			if (!_insert_range<true>(index, length))
				return false;
			return true;
		}

		[[nodiscard]] PBOS_FORCEINLINE bool insert(size_t index, T &&data) {
			T *gap = (T *)_insert_range<false>(index, 1);

			if (!gap)
				return false;

			construct_at<T>(gap, std::move(data));

			return true;
		}

		[[nodiscard]] PBOS_FORCEINLINE bool push_front(T &&data) {
			return insert(0, std::move(data));
		}

		[[nodiscard]] PBOS_FORCEINLINE bool push_back(T &&data) {
			return insert(_length, std::move(data));
		}

		[[nodiscard]] PBOS_FORCEINLINE T &front() {
			return at(0);
		}

		[[nodiscard]] PBOS_FORCEINLINE const T &front() const {
			return at(0);
		}

		[[nodiscard]] PBOS_FORCEINLINE T &back() {
			return at(_length - 1);
		}

		[[nodiscard]] PBOS_FORCEINLINE const T &back() const {
			return at(_length - 1);
		}

		[[nodiscard]] PBOS_FORCEINLINE bool pop_back_and_shrink() {
			return resize_uninit_and_shrink(_length - 1);
		}

		PBOS_FORCEINLINE void pop_back() {
			resize_uninit(_length - 1);
		}

		[[nodiscard]] PBOS_FORCEINLINE bool pop_front_and_shrink() {
			T front_data = std::move(front());
			size_t len = _length - 1;
			_move_data(_data, _data + 1, len);
			if (!resize(len)) {
				_move_data(_data + 1, _data, len);
				construct_at(&_data[0], std::move(front_data));
				return false;
			}
			return true;
		}

		PBOS_FORCEINLINE void pop_front() {
			_move_data(_data, _data + 1, _length - 1);
			resize(_length - 1);
		}

		PBOS_FORCEINLINE allocator_t *allocator() const {
			return _allocator.get();
		}

		PBOS_FORCEINLINE T *data() {
			return _data;
		}

		PBOS_FORCEINLINE const T *data() const {
			return _data;
		}

		PBOS_FORCEINLINE iterator begin() {
			return _data;
		}

		PBOS_FORCEINLINE iterator end() {
			return _data + _length;
		}

		PBOS_FORCEINLINE const_iterator begin() const {
			return _data;
		}

		PBOS_FORCEINLINE const_iterator end() const {
			return _data + _length;
		}
	};
}

#endif
