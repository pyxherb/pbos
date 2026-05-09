#ifndef _PBOS_KFXX_UNIQUE_PTR_HH_
#define _PBOS_KFXX_UNIQUE_PTR_HH_

#include <memory>
#include <type_traits>
#include <utility>
#include "basedefs.hh"

namespace kfxx {
	template <typename T, typename D>
	struct unique_ptr_t {
	private:
		static_assert(std::is_invocable_v<D, T *>, "The deleter is not invocable");
		T *_ptr;
		[[no_unique_address]] D _deleter;

		using this_t = kfxx::unique_ptr_t<T, D>;

	public:
		PBOS_FORCEINLINE void reset() noexcept {
			if (_ptr) {
				_deleter(_ptr);
				_ptr = nullptr;
			}
		}

		PBOS_FORCEINLINE unique_ptr_t(T *ptr, D deleter = {}) noexcept : _ptr(ptr), _deleter(std::move(deleter)) {
		}
		unique_ptr_t(const this_t &other) noexcept = delete;
		PBOS_FORCEINLINE unique_ptr_t(this_t &&other) noexcept : _ptr(other._ptr), _deleter(other._deleter) {
			other._ptr = nullptr;
		}
		PBOS_FORCEINLINE ~unique_ptr_t() {
			reset();
		}

		PBOS_FORCEINLINE this_t &operator=(T *ptr) noexcept {
			reset();
			_ptr = ptr;
			return *this;
		}
		this_t &operator=(const this_t &other) = delete;
		PBOS_FORCEINLINE this_t &operator=(this_t &&other) noexcept {
			reset();

			_deleter = std::move(other._deleter);
			_ptr = other._ptr;
			other._ptr = nullptr;

			return *this;
		}

		PBOS_FORCEINLINE T *get() const noexcept {
			return _ptr;
		}
		PBOS_FORCEINLINE T *&get_ref() noexcept {
			reset();
			return _ptr;
		}
		PBOS_FORCEINLINE T *&get_ref_without_release() noexcept {
			return _ptr;
		}
		PBOS_FORCEINLINE T *const &get_Ref_without_release() const noexcept {
			return _ptr;
		}
		PBOS_FORCEINLINE T **get_addr() noexcept {
			reset();
			return &_ptr;
		}
		PBOS_FORCEINLINE T **get_addr_without_release() noexcept {
			return &_ptr;
		}
		PBOS_FORCEINLINE T *const *get_addr_without_release() const noexcept {
			return &_ptr;
		}
		PBOS_FORCEINLINE T *release() noexcept {
			T *ptr = _ptr;
			_ptr = nullptr;
			return ptr;
		}
		PBOS_FORCEINLINE T *operator->() const noexcept {
			return _ptr;
		}

		PBOS_FORCEINLINE bool operator<(const T *rhs) const noexcept {
			return _ptr < rhs;
		}

		PBOS_FORCEINLINE bool operator>(const T *rhs) const noexcept {
			return _ptr > rhs;
		}

		PBOS_FORCEINLINE bool operator==(const T *rhs) const noexcept {
			return _ptr == rhs;
		}

		PBOS_FORCEINLINE bool operator!=(const T *rhs) const noexcept {
			return _ptr != rhs;
		}

		PBOS_FORCEINLINE bool operator<(const this_t &rhs) const noexcept {
			return _ptr < rhs._ptr;
		}

		PBOS_FORCEINLINE bool operator>(const this_t &rhs) const noexcept {
			return _ptr > rhs._ptr;
		}

		PBOS_FORCEINLINE operator bool() const noexcept {
			return _ptr;
		}

		PBOS_FORCEINLINE bool operator==(const this_t &rhs) const noexcept {
			return _ptr == rhs._ptr;
		}

		PBOS_FORCEINLINE bool operator!=(const this_t &rhs) const noexcept {
			return _ptr != rhs._ptr;
		}
	};
}

#endif
