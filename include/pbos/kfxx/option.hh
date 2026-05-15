#ifndef _PBOS_KFXX_OPTIONAL_H_
#define _PBOS_KFXX_OPTIONAL_H_

#include "basedefs.hh"
#include "utils.hh"
#include <type_traits>
#include <pbos/kd/assert.h>

namespace kfxx {
	struct NullOption {
	};

	constexpr static NullOption NULL_OPTION;

	template <typename T>
	class Option final {
	private:
		alignas(T) char _data[sizeof(T)];
		bool _has_value = false;

	public:
		using value_type = T;

		PBOS_FORCEINLINE void reset() noexcept {
			if (_has_value) {
				if (std::is_destructible_v<T>) {
					destroy_at<T>((T *)_data);
				}
			}
			_has_value = false;
		}

		PBOS_FORCEINLINE void set_value(T &&data) noexcept {
			reset();
			construct_at<T>(((T *)_data), std::move(data));
			_has_value = true;
		}

		PBOS_FORCEINLINE void set_value(NullOption) noexcept {
			reset();
		}

		PBOS_FORCEINLINE bool has_value() const noexcept {
			return _has_value;
		}

		PBOS_FORCEINLINE T &value() noexcept {
			kd_assert(has_value());
			return *((T *)_data);
		}

		PBOS_FORCEINLINE const T &value() const noexcept {
			kd_assert(has_value());
			return *((const T *)_data);
		}

		PBOS_FORCEINLINE T move() noexcept {
			kd_assert(has_value());
			_has_value = false;
			return std::move(*((T *)_data));
		}

		PBOS_FORCEINLINE T &operator*() noexcept {
			kd_assert(has_value());
			return value();
		}

		PBOS_FORCEINLINE const T &operator*() const noexcept {
			kd_assert(has_value());
			return value();
		}

		PBOS_FORCEINLINE std::remove_reference_t<T> *operator->() noexcept {
			kd_assert(has_value());
			return &value();
		}

		PBOS_FORCEINLINE std::remove_reference_t<const T> *operator->() const noexcept {
			kd_assert(has_value());
			return &value();
		}

		PBOS_FORCEINLINE Option() noexcept {
		}

		PBOS_FORCEINLINE ~Option() {
			reset();
		}

		PBOS_FORCEINLINE Option(T &&data) noexcept {
			set_value(std::move(data));
		}

		PBOS_FORCEINLINE Option(NullOption) noexcept {
		}

		PBOS_FORCEINLINE Option(Option<T> &&rhs) noexcept {
			if (rhs.has_value()) {
				set_value(std::move(*((T *)rhs._data)));
			}
		}

		PBOS_FORCEINLINE Option<T> &operator=(Option<T> &&rhs) noexcept {
			reset();

			if (rhs.has_value()) {
				set_value(std::move(*((T *)rhs._data)));
			}
			return *this;
		}
	};

	template <typename T>
	class Option<T &> final {
	private:
		T *_data;
		bool _has_value = false;

	public:
		using value_type = T &;

		PBOS_FORCEINLINE void reset() noexcept {
			_has_value = false;
		}

		PBOS_FORCEINLINE void set_value_ref(T &data) noexcept {
			reset();
			_data = &data;
			_has_value = true;
		}

		PBOS_FORCEINLINE void set_value(NullOption) noexcept {
			reset();
		}

		PBOS_FORCEINLINE bool has_value() const noexcept {
			return _has_value;
		}

		PBOS_FORCEINLINE T &value() noexcept {
			kd_assert(has_value());
			return *_data;
		}

		PBOS_FORCEINLINE const T &value() const noexcept {
			kd_assert(has_value());
			return *((const T *)_data);
		}

		PBOS_FORCEINLINE T move() noexcept {
			kd_assert(has_value());
			_has_value = false;
			return std::move(*_data);
		}

		PBOS_FORCEINLINE T &operator*() noexcept {
			kd_assert(has_value());
			return value();
		}

		PBOS_FORCEINLINE const T &operator*() const noexcept {
			kd_assert(has_value());
			return value();
		}

		PBOS_FORCEINLINE std::remove_reference_t<T> *operator->() noexcept {
			kd_assert(has_value());
			return &value();
		}

		PBOS_FORCEINLINE std::remove_reference_t<const T> *operator->() const noexcept {
			kd_assert(has_value());
			return &value();
		}

		PBOS_FORCEINLINE Option() noexcept {
		}

		PBOS_FORCEINLINE ~Option() {
			reset();
		}

		PBOS_FORCEINLINE Option(T &data) noexcept {
			set_value_ref(data);
		}

		PBOS_FORCEINLINE Option(NullOption) noexcept {
		}

		PBOS_FORCEINLINE Option(Option<T> &&rhs) noexcept {
			if (rhs.has_value()) {
				set_value_ref(**((std::remove_reference_t<T> **)rhs._data));
			}
		}

		PBOS_FORCEINLINE Option<T> &operator=(Option<T> &&rhs) noexcept {
			reset();

			if (rhs.has_value()) {
				set_value_ref(**((std::remove_reference_t<T> **)rhs._data));
			}
			return *this;
		}
	};
}

#endif
