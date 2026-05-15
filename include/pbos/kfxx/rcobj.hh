#ifndef _PEFF_BASE_RCOBJ_H_
#define _PEFF_BASE_RCOBJ_H_

#include <pbos/kd/assert.h>
#include <cstddef>
#include <type_traits>
#include "basedefs.hh"

namespace kfxx {
#if __cplusplus >= 202002L
	template <typename T>
	concept RcObjectTrait = requires(T *rc_object) {
		rc_object->inc_ref();
		rc_object->dec_ref();
	};
#endif

	template <typename T>
	class RcObjectPtr {
	private:
		using ThisType = RcObjectPtr<T>;

		T *_ptr = nullptr;

		PBOS_FORCEINLINE void _set_and_inc_ref(T *_ptr)
			PBOS_REQUIRES_CONCEPT(RcObjectTrait<T>) {
			_ptr->inc_ref();
			this->_ptr = _ptr;
		}

	public:
		PBOS_FORCEINLINE void reset() noexcept
			PBOS_REQUIRES_CONCEPT(RcObjectTrait<T>) {
			if (_ptr) {
				_ptr->dec_ref();
			}
			_ptr = nullptr;
		}

		PBOS_FORCEINLINE RcObjectPtr() : _ptr(nullptr) {
		}
		PBOS_FORCEINLINE RcObjectPtr(T *ptr) noexcept : _ptr(nullptr) {
			if (ptr) {
				_set_and_inc_ref(ptr);
			}
		}
		PBOS_FORCEINLINE RcObjectPtr(const ThisType &other) noexcept : _ptr(nullptr) {
			if (other._ptr) {
				_set_and_inc_ref(other._ptr);
			}
		}
		PBOS_FORCEINLINE RcObjectPtr(ThisType &&other) noexcept {
			_ptr = other._ptr;
			other._ptr = nullptr;
		}
		PBOS_FORCEINLINE ~RcObjectPtr() {
			reset();
		}

		PBOS_FORCEINLINE RcObjectPtr<T> &operator=(T *_ptr) noexcept {
			reset();
			if (_ptr) {
				_set_and_inc_ref(_ptr);
			}
			return *this;
		}
		PBOS_FORCEINLINE RcObjectPtr<T> &operator=(const ThisType &other) noexcept {
			reset();
			_set_and_inc_ref(other._ptr);
			return *this;
		}
		PBOS_FORCEINLINE RcObjectPtr<T> &operator=(ThisType &&other) noexcept {
			reset();

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

	template <typename T, typename IncRef, typename DecRef>
	class CustomRcPtr {
	private:
		using ThisType = CustomRcPtr<T, IncRef, DecRef>;

		T *_ptr = nullptr;
		[[no_unique_address]] IncRef _inc_ref;
		[[no_unique_address]] DecRef _dec_ref;

		PBOS_FORCEINLINE void _set_and_inc_ref(T *ptr) {
			_inc_ref(ptr);
			this->_ptr = ptr;
		}

	public:
		PBOS_FORCEINLINE void reset() noexcept {
			if (_ptr)
				_dec_ref(_ptr);
			_ptr = nullptr;
		}

		PBOS_FORCEINLINE CustomRcPtr() : _ptr(nullptr) {
		}
		PBOS_FORCEINLINE CustomRcPtr(T *ptr) noexcept : _ptr(nullptr) {
			if (ptr) {
				_set_and_inc_ref(ptr);
			}
		}
		PBOS_FORCEINLINE CustomRcPtr(const ThisType &other) noexcept : _ptr(nullptr) {
			if (other._ptr) {
				_set_and_inc_ref(other._ptr);
			}
		}
		PBOS_FORCEINLINE CustomRcPtr(ThisType &&other) noexcept {
			_ptr = other._ptr;
			other._ptr = nullptr;
		}
		PBOS_FORCEINLINE ~CustomRcPtr() {
			reset();
		}

		PBOS_FORCEINLINE CustomRcPtr &operator=(T *_ptr) noexcept {
			reset();
			if (_ptr) {
				_set_and_inc_ref(_ptr);
			}
			return *this;
		}
		PBOS_FORCEINLINE CustomRcPtr &operator=(const ThisType &other) noexcept {
			reset();
			_set_and_inc_ref(other._ptr);
			return *this;
		}
		PBOS_FORCEINLINE CustomRcPtr &operator=(ThisType &&other) noexcept {
			reset();

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
		PBOS_FORCEINLINE T **operator&() noexcept {
			reset();
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

		PBOS_FORCEINLINE bool operator<(const ThisType &rhs) const noexcept {
			return _ptr < rhs._ptr;
		}

		PBOS_FORCEINLINE bool operator>(const ThisType &rhs) const noexcept {
			return _ptr > rhs._ptr;
		}

		PBOS_FORCEINLINE bool operator<(const T *rhs) const noexcept {
			return _ptr < rhs;
		}

		PBOS_FORCEINLINE bool operator>(const T *rhs) const noexcept {
			return _ptr < rhs;
		}

		PBOS_FORCEINLINE bool operator==(const T *rhs) const noexcept {
			return _ptr == rhs;
		}

		PBOS_FORCEINLINE bool operator!=(const T *rhs) const noexcept {
			return _ptr != rhs;
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
