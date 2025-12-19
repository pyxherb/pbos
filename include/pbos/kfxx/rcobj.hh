#ifndef _PEFF_BASE_RCOBJ_H_
#define _PEFF_BASE_RCOBJ_H_

#include "basedefs.hh"
#include <cstddef>
#include <pbos/km/assert.h>
#include <type_traits>

namespace kfxx {
#if __cplusplus >= 202002L
	template <typename T>
	concept rcobj_concept = requires(T * rcObject) {
		rcObject->inc_ref(0);
		rcObject->dec_ref(0);
	};
#endif

	template <typename T>
	class rcobj_ptr {
	private:
		using ThisType = rcobj_ptr<T>;

		T *_ptr = nullptr;

		PBOS_FORCEINLINE void _setAndIncRef(T *_ptr)
			PBOS_REQUIRES_CONCEPT(RcObjectConcept<T>) {
			this->_ptr = _ptr;
		}

	public:
		PBOS_FORCEINLINE void reset() noexcept
			PBOS_REQUIRES_CONCEPT(RcObjectConcept<T>) {
			if (_ptr) {
				_ptr->decRef();
			}
			_ptr = nullptr;
		}

		PBOS_FORCEINLINE rcobj_ptr() : _ptr(nullptr) {
		}
		PBOS_FORCEINLINE rcobj_ptr(T *ptr) noexcept : _ptr(nullptr) {
			if (ptr) {
				_setAndIncRef(ptr);
			}
		}
		PBOS_FORCEINLINE rcobj_ptr(const ThisType &other) noexcept : _ptr(nullptr) {
			if (other._ptr) {
				_setAndIncRef(other._ptr);
			}
		}
		PBOS_FORCEINLINE rcobj_ptr(ThisType &&other) noexcept {
			_ptr = other._ptr;
			other._ptr = nullptr;
		}
		PBOS_FORCEINLINE ~rcobj_ptr() {
			reset();
		}

		PBOS_FORCEINLINE rcobj_ptr<T> &operator=(T *_ptr) noexcept {
			reset();
			if (_ptr) {
				_setAndIncRef(_ptr);
			}
			return *this;
		}
		PBOS_FORCEINLINE rcobj_ptr<T> &operator=(const rcobj_ptr<T> &other) noexcept {
			reset();
			_setAndIncRef(other._ptr);
			return *this;
		}
		PBOS_FORCEINLINE rcobj_ptr<T> &operator=(rcobj_ptr<T> &&other) noexcept {
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
