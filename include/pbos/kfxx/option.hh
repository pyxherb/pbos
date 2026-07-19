#ifndef _PBOS_KFXX_OPTION_HH_
#define _PBOS_KFXX_OPTION_HH_

#include <pbos/kd/assert.h>
#include <type_traits>
#include "basedefs.hh"
#include "utils.hh"

namespace kfxx {
	struct nullopt_t {
	};

	constexpr static nullopt_t nullopt;

	template <typename T>
	class option_t final {
	private:
		alignas(T) char _data[sizeof(T)];
		bool _has_value = false;

	public:
		using value_type = T;

		static_assert(std::is_move_constructible_v<T>, "The type must be move constructible");

		PBOS_FORCEINLINE void reset() noexcept {
			if (_has_value) {
				if (std::is_destructible_v<T>) {
					kfxx::destroy_at<T>((T *)_data);
				}
			}
			_has_value = false;
		}

		PBOS_FORCEINLINE void set_value(T &&data) noexcept {
			reset();
			kfxx::construct_at<T>(((T *)_data), std::move(data));
			_has_value = true;
		}

		PBOS_FORCEINLINE void set_value(nullopt_t) noexcept {
			reset();
		}

		PBOS_FORCEINLINE bool has_value() const noexcept {
			return _has_value;
		}

		PBOS_FORCEINLINE operator bool() const noexcept {
			return _has_value;
		}

		PBOS_FORCEINLINE T &value() & noexcept {
			kd_assert(has_value());
			return *((T *)_data);
		}

		PBOS_FORCEINLINE const T &value() const & noexcept {
			kd_assert(has_value());
			return *((const T *)_data);
		}

		PBOS_FORCEINLINE T value() && noexcept {
			kd_assert(has_value());
			_has_value = false;
			return std::move(*((T *)_data));
		}

		PBOS_FORCEINLINE T move() noexcept {
			kd_assert(has_value());
			_has_value = false;
			return std::move(*((T *)_data));
		}

		PBOS_FORCEINLINE const T &operator*() const & noexcept {
			return value();
		}

		PBOS_FORCEINLINE T &operator*() & noexcept {
			return value();
		}

		PBOS_FORCEINLINE T operator*() && noexcept {
			return value();
		}

		PBOS_FORCEINLINE T *operator->() noexcept {
			kd_assert(has_value());
			return &value();
		}

		PBOS_FORCEINLINE const T *operator->() const noexcept {
			kd_assert(has_value());
			return &value();
		}

		PBOS_FORCEINLINE explicit option_t() noexcept {
		}

		PBOS_FORCEINLINE ~option_t() {
			reset();
		}

		PBOS_FORCEINLINE option_t(const T &data) noexcept {
			T copied_data = data;
			set_value(std::move(copied_data));
		}

		PBOS_FORCEINLINE option_t(T &&data) noexcept {
			set_value(std::move(data));
		}

		PBOS_FORCEINLINE option_t(nullopt_t) noexcept {
		}

		PBOS_FORCEINLINE option_t(option_t<T> &&rhs) noexcept {
			if (rhs.has_value()) {
				set_value(std::move(*((T *)rhs._data)));
				rhs._has_value = false;
			}
		}

		PBOS_FORCEINLINE option_t<T> &operator=(option_t<T> &&rhs) noexcept {
			reset();

			if (rhs.has_value()) {
				set_value(std::move(*((T *)rhs._data)));
				rhs._has_value = false;
			}
			return *this;
		}

		PBOS_FORCEINLINE option_t<T> &operator=(const T &rhs) noexcept {
			reset();

			T copied_data = rhs;
			set_value(std::move(copied_data));
			return *this;
		}

		PBOS_FORCEINLINE option_t<T> &operator=(T &&rhs) noexcept {
			reset();

			set_value(std::move(rhs));
			return *this;
		}
	};

	template <typename T>
	class option_t<T &> final {
	private:
		using V = std::remove_reference_t<T>;
		V *_data = nullptr;
		using ThisType = option_t<T &>;

	public:
		using value_type = T &;

		PBOS_FORCEINLINE void reset() noexcept {
			_data = nullptr;
		}

		PBOS_FORCEINLINE void set_value_ref(V &data) noexcept {
			reset();
			_data = &data;
		}

		PBOS_FORCEINLINE void set_value(nullopt_t) noexcept {
			reset();
		}

		PBOS_FORCEINLINE bool has_value() const noexcept {
			return _data;
		}

		PBOS_FORCEINLINE operator bool() const noexcept {
			return _data;
		}

		PBOS_FORCEINLINE V &value() noexcept {
			kd_assert(has_value());
			return *_data;
		}

		PBOS_FORCEINLINE const V &value() const noexcept {
			kd_assert(has_value());
			return *((const V *)_data);
		}

		PBOS_FORCEINLINE V move() noexcept {
			kd_assert(has_value());
			V v = std::move(*_data);
			_data = nullptr;
			return v;
		}

		PBOS_FORCEINLINE V &operator*() noexcept {
			kd_assert(has_value());
			return value();
		}

		PBOS_FORCEINLINE const V &operator*() const noexcept {
			kd_assert(has_value());
			return value();
		}

		PBOS_FORCEINLINE V *operator->() noexcept {
			kd_assert(has_value());
			return &value();
		}

		PBOS_FORCEINLINE const V *operator->() const noexcept {
			kd_assert(has_value());
			return &value();
		}

		PBOS_FORCEINLINE explicit option_t() noexcept {
		}

		PBOS_FORCEINLINE ~option_t() {
			reset();
		}

		PBOS_FORCEINLINE option_t(V &data) noexcept {
			set_value_ref(data);
		}

		PBOS_FORCEINLINE option_t(nullopt_t) noexcept {
		}

		PBOS_FORCEINLINE option_t(ThisType &&rhs) noexcept {
			if (rhs.has_value()) {
				set_value_ref(**((std::remove_reference_t<T> **)rhs._data));
				rhs._data = nullptr;
			}
		}

		PBOS_FORCEINLINE ThisType &operator=(option_t<T> &&rhs) noexcept {
			reset();

			if (rhs.has_value()) {
				set_value_ref(**((std::remove_reference_t<T> **)rhs._data));
				rhs._data = nullptr;
			}
			return *this;
		}

		PBOS_FORCEINLINE ThisType &operator=(V &rhs) noexcept {
			reset();
			set_value_ref(rhs);
			return *this;
		}
	};

	template <typename T>
	class option_t<const T &> final {
	private:
		using V = const std::remove_reference_t<T>;
		using ThisType = option_t<const T &>;
		V *_data = nullptr;

	public:
		using value_type = const T &;

		PBOS_FORCEINLINE void reset() noexcept {
			_data = nullptr;
		}

		PBOS_FORCEINLINE void set_value_ref(V &data) noexcept {
			reset();
			_data = &data;
		}

		PBOS_FORCEINLINE void set_value(nullopt_t) noexcept {
			reset();
		}

		PBOS_FORCEINLINE bool has_value() const noexcept {
			return _data;
		}

		PBOS_FORCEINLINE operator bool() const noexcept {
			return _data;
		}

		PBOS_FORCEINLINE V &value() noexcept {
			kd_assert(has_value());
			return *_data;
		}

		PBOS_FORCEINLINE V &value() const noexcept {
			kd_assert(has_value());
			return *((V *)_data);
		}

		PBOS_FORCEINLINE V move() noexcept {
			kd_assert(has_value());
			V v = std::move(*_data);
			_data = nullptr;
			return v;
		}

		PBOS_FORCEINLINE V &operator*() noexcept {
			kd_assert(has_value());
			return value();
		}

		PBOS_FORCEINLINE V &operator*() const noexcept {
			kd_assert(has_value());
			return value();
		}

		PBOS_FORCEINLINE V *operator->() noexcept {
			kd_assert(has_value());
			return &value();
		}

		PBOS_FORCEINLINE V *operator->() const noexcept {
			kd_assert(has_value());
			return &value();
		}

		PBOS_FORCEINLINE explicit option_t() noexcept {
		}

		PBOS_FORCEINLINE ~option_t() {
			reset();
		}

		PBOS_FORCEINLINE option_t(V &data) noexcept {
			set_value_ref(data);
		}

		PBOS_FORCEINLINE option_t(nullopt_t) noexcept {
		}

		PBOS_FORCEINLINE option_t(ThisType &&rhs) noexcept {
			if (rhs.has_value()) {
				set_value_ref(**((std::remove_reference_t<T> **)rhs._data));
				rhs._data = nullptr;
			}
		}

		PBOS_FORCEINLINE ThisType &operator=(ThisType &&rhs) noexcept {
			reset();

			if (rhs.has_value()) {
				set_value_ref(**((std::remove_reference_t<T> **)rhs._data));
				rhs._data = nullptr;
			}
			return *this;
		}

		PBOS_FORCEINLINE ThisType &operator=(V &rhs) noexcept {
			reset();
			set_value_ref(rhs);
			return *this;
		}
	};

	template <typename T, size_t length>
	class option_array_t final {
	private:
		alignas(T) char _data[sizeof(T) * length];
		bool _has_value[length] = { false };

	public:
		using value_type = T;

		PBOS_FORCEINLINE void reset(size_t index) noexcept {
			if (_has_value[index]) {
				if (std::is_destructible_v<T>) {
					kfxx::destroy_at<T>((T *)(&_data[index * sizeof(T)]));
				}
			}
			_has_value[index] = false;
		}

		PBOS_FORCEINLINE void set_value(size_t index, T &&data) noexcept {
			kd_assert(index < length);
			reset(index);
			kfxx::construct_at<T>(((T *)(&_data[index * sizeof(T)])), std::move(data));
			_has_value[index] = true;
		}

		PBOS_FORCEINLINE void set_value(size_t index, nullopt_t) noexcept {
			kd_assert(index < length);
			reset(index);
		}

		PBOS_FORCEINLINE bool has_value(size_t index) const noexcept {
			kd_assert(index < length);
			return _has_value[index];
		}

		PBOS_FORCEINLINE T &value(size_t index) & noexcept {
			kd_assert(index < length);
			kd_assert(has_value(index));
			return *((T *)(&_data[index * sizeof(T)]));
		}

		PBOS_FORCEINLINE const T &value(size_t index) const & noexcept {
			kd_assert(index < length);
			kd_assert(has_value(index));
			return *((const T *)(&_data[index * sizeof(T)]));
		}

		PBOS_FORCEINLINE T value(size_t index) && noexcept {
			kd_assert(index < length);
			kd_assert(has_value(index));
			_has_value = false;
			return std::move(*((T *)(&_data[index * sizeof(T)])));
		}

		PBOS_FORCEINLINE T move(size_t index) noexcept {
			kd_assert(index < length);
			kd_assert(has_value(index));
			_has_value[index] = false;
			return std::move(*((T *)(&_data[index * sizeof(T)])));
		}

		PBOS_FORCEINLINE option_array_t() noexcept {
		}

		PBOS_FORCEINLINE ~option_array_t() {
			for (size_t i = 0; i < length; ++i)
				reset(i);
		}

		PBOS_FORCEINLINE option_array_t(option_array_t<T, length> &&rhs) noexcept {
			for (size_t i = 0; i < length; ++i) {
				if (rhs.has_value(i)) {
					set_value(i, std::move(*((T *)&rhs._data[i * sizeof(T)])));
					rhs._has_value[i] = false;
				}
			}
		}

		PBOS_FORCEINLINE option_array_t<T, length> &operator=(option_array_t<T, length> &&rhs) noexcept {
			if (&rhs == this)
				return *this;
			for (size_t i = 0; i < length; ++i) {
				reset(i);
				if (rhs.has_value(i)) {
					set_value(i, std::move(*((T *)&rhs._data[i * sizeof(T)])));
					rhs._has_value[i] = false;
				}
			}
			return *this;
		}
	};
}

#endif
