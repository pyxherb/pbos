#ifndef _PBOS_KFXX_OPTION_H_
#define _PBOS_KFXX_OPTION_H_

#include "basedefs.hh"
#include "utils.hh"
#include <type_traits>

namespace kfxx {
	struct NullOption {
	};

	constexpr static NullOption NULL_OPTION;

	template <typename T>
	class Option final {
	private:
		alignas(T) char _data[sizeof(T)];
		bool _hasValue = false;

	public:
		using value_type = T;

		PB_FORCEINLINE void reset() noexcept {
			if (_hasValue) {
				if (std::is_destructible_v<T>) {
					destroyAt<T>((T *)_data);
				}
			}
			_hasValue = false;
		}

		PB_FORCEINLINE void setValue(T &&data) noexcept {
			reset();
			constructAt<T>(((T *)_data), std::move(data));
			_hasValue = true;
		}

		PB_FORCEINLINE void setValue(NullOption) noexcept {
			reset();
		}

		PB_FORCEINLINE bool hasValue() const noexcept {
			return _hasValue;
		}

		PB_FORCEINLINE T &value() noexcept {
			assert(hasValue());
			return *((T *)_data);
		}

		PB_FORCEINLINE const T &value() const noexcept {
			assert(hasValue());
			return *((const T *)_data);
		}

		PB_FORCEINLINE T move() noexcept {
			assert(hasValue());
			_hasValue = false;
			return std::move(*((T *)_data));
		}

		PB_FORCEINLINE T &operator*() noexcept {
			assert(hasValue());
			return value();
		}

		PB_FORCEINLINE const T &operator*() const noexcept {
			assert(hasValue());
			return value();
		}

		PB_FORCEINLINE std::remove_reference_t<T> *operator->() noexcept {
			assert(hasValue());
			return &value();
		}

		PB_FORCEINLINE std::remove_reference_t<const T> *operator->() const noexcept {
			assert(hasValue());
			return &value();
		}

		PB_FORCEINLINE Option() noexcept {
		}

		PB_FORCEINLINE ~Option() {
			reset();
		}

		PB_FORCEINLINE Option(T &&data) noexcept {
			setValue(std::move(data));
		}

		PB_FORCEINLINE Option(NullOption) noexcept {
		}

		PB_FORCEINLINE Option(Option<T> &&rhs) noexcept {
			if (rhs.hasValue()) {
				setValue(std::move(*((T *)rhs._data)));
			}
		}

		PB_FORCEINLINE Option<T> &operator=(Option<T> &&rhs) noexcept {
			reset();

			if (rhs.hasValue()) {
				setValue(std::move(*((T *)rhs._data)));
			}
			return *this;
		}
	};

	template <typename T>
	class Option<T &> final {
	private:
		T *_data;
		bool _hasValue = false;

	public:
		using value_type = T &;

		PB_FORCEINLINE void reset() noexcept {
			_hasValue = false;
		}

		PB_FORCEINLINE void setValueRef(T &data) noexcept {
			reset();
			_data = &data;
			_hasValue = true;
		}

		PB_FORCEINLINE void setValue(NullOption) noexcept {
			reset();
		}

		PB_FORCEINLINE bool hasValue() const noexcept {
			return _hasValue;
		}

		PB_FORCEINLINE T &value() noexcept {
			assert(hasValue());
			return *_data;
		}

		PB_FORCEINLINE const T &value() const noexcept {
			assert(hasValue());
			return *((const T *)_data);
		}

		PB_FORCEINLINE T move() noexcept {
			assert(hasValue());
			_hasValue = false;
			return std::move(*_data);
		}

		PB_FORCEINLINE T &operator*() noexcept {
			assert(hasValue());
			return value();
		}

		PB_FORCEINLINE const T &operator*() const noexcept {
			assert(hasValue());
			return value();
		}

		PB_FORCEINLINE std::remove_reference_t<T> *operator->() noexcept {
			assert(hasValue());
			return &value();
		}

		PB_FORCEINLINE std::remove_reference_t<const T> *operator->() const noexcept {
			assert(hasValue());
			return &value();
		}

		PB_FORCEINLINE Option() noexcept {
		}

		PB_FORCEINLINE ~Option() {
			reset();
		}

		PB_FORCEINLINE Option(T &data) noexcept {
			setValueRef(data);
		}

		PB_FORCEINLINE Option(NullOption) noexcept {
		}

		PB_FORCEINLINE Option(Option<T> &&rhs) noexcept {
			if (rhs.hasValue()) {
				setValueRef(**((std::remove_reference_t<T> **)rhs._data));
			}
		}

		PB_FORCEINLINE Option<T> &operator=(Option<T> &&rhs) noexcept {
			reset();

			if (rhs.hasValue()) {
				setValueRef(**((std::remove_reference_t<T> **)rhs._data));
			}
			return *this;
		}
	};
}

#endif
