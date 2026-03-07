#ifndef _PEFF_BASE_RCOBJ_H_
#define _PEFF_BASE_RCOBJ_H_

#include "basedefs.hh"
#include <cstddef>
#include <pbos/km/assert.h>
#include <type_traits>

namespace kfxx {
#if __cplusplus >= 202002L
	template <typename T>
	concept rc_object_trait = requires(T * rcObject) {
		rcObject->inc_ref(0);
		rcObject->dec_ref(0);
	};
#endif

	template <typename T>
	class rc_object_ptr {
	private:
		using ThisType = rc_object_ptr<T>;

		T *_ptr = nullptr;

		PBOS_FORCEINLINE void _set_and_inc_ref(T *_ptr)
			PBOS_REQUIRES_CONCEPT(rc_object_trait<T>) {
			this->_ptr = _ptr;
		}

	public:
		PBOS_FORCEINLINE void reset() noexcept
			PBOS_REQUIRES_CONCEPT(rc_object_trait<T>) {
			if (_ptr) {
				_ptr->decRef();
			}
			_ptr = nullptr;
		}

		PBOS_FORCEINLINE rc_object_ptr() : _ptr(nullptr) {
		}
		PBOS_FORCEINLINE rc_object_ptr(T *ptr) noexcept : _ptr(nullptr) {
			if (ptr) {
				_set_and_inc_ref(ptr);
			}
		}
		PBOS_FORCEINLINE rc_object_ptr(const ThisType &other) noexcept : _ptr(nullptr) {
			if (other._ptr) {
				_set_and_inc_ref(other._ptr);
			}
		}
		PBOS_FORCEINLINE rc_object_ptr(ThisType &&other) noexcept {
			_ptr = other._ptr;
			other._ptr = nullptr;
		}
		PBOS_FORCEINLINE ~rc_object_ptr() {
			reset();
		}

		PBOS_FORCEINLINE rc_object_ptr<T> &operator=(T *_ptr) noexcept {
			reset();
			if (_ptr) {
				_set_and_inc_ref(_ptr);
			}
			return *this;
		}
		PBOS_FORCEINLINE rc_object_ptr<T> &operator=(const rc_object_ptr<T> &other) noexcept {
			reset();
			_set_and_inc_ref(other._ptr);
			return *this;
		}
		PBOS_FORCEINLINE rc_object_ptr<T> &operator=(rc_object_ptr<T> &&other) noexcept {
			reset();

			_ptr = other._ptr;

			other._ptr = nullptr;

			return *this;
		}

		PBOS_FORCEINLINE T *get() const noexcept {
			return _ptr;
		}
		PBOS_FORCEINLINE T *&getRef() noexcept {
			reset();
			return _ptr;
		}
		PBOS_FORCEINLINE T *&getRefWithoutRelease() noexcept {
			return _ptr;
		}
		PBOS_FORCEINLINE T *const &getRefWithoutRelease() const noexcept {
			return _ptr;
		}
		PBOS_FORCEINLINE T **getAddressOf() noexcept {
			reset();
			return &_ptr;
		}
		PBOS_FORCEINLINE T **getAddressOfWithoutRelease() noexcept {
			return &_ptr;
		}
		PBOS_FORCEINLINE T *const *getAddressOfWithoutRelease() const noexcept {
			return &_ptr;
		}
		PBOS_FORCEINLINE T *operator->() const noexcept {
			return _ptr;
		}

		PBOS_FORCEINLINE bool operator<(const ThisType &rhs) const noexcept {
			return _ptr < rhs._ptr;
		}

		PBOS_FORCEINLINE operator bool() const noexcept {
			return _ptr;
		}

		PBOS_FORCEINLINE bool operator==(const ThisType &rhs) const noexcept {
			return _ptr == rhs._ptr;
		}

		PBOS_FORCEINLINE bool operator!=(const ThisType &rhs) const noexcept {
			return _ptr != rhs._ptr;
		}
	};
}

#endif
