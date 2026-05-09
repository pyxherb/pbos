#ifndef _PEFF_BASE_RCOBJ_H_
#define _PEFF_BASE_RCOBJ_H_

#include <pbos/km/assert.h>
#include <cstddef>
#include <type_traits>
#include "basedefs.hh"

namespace kfxx {
#if __cplusplus >= 202002L
	template <typename T>
	concept rc_object_trait = requires(T *rc_object) {
		rc_object->inc_ref();
		rc_object->dec_ref();
	};
#endif

	template <typename T>
	class rc_object_ptr_t {
	private:
		using this_t = rc_object_ptr_t<T>;

		T *_ptr = nullptr;

		PBOS_FORCEINLINE void _set_and_inc_ref(T *_ptr)
			PBOS_REQUIRES_CONCEPT(rc_object_trait<T>) {
			_ptr->inc_ref();
			this->_ptr = _ptr;
		}

	public:
		PBOS_FORCEINLINE void reset() noexcept
			PBOS_REQUIRES_CONCEPT(rc_object_trait<T>) {
			if (_ptr) {
				_ptr->dec_ref();
			}
			_ptr = nullptr;
		}

		PBOS_FORCEINLINE rc_object_ptr_t() : _ptr(nullptr) {
		}
		PBOS_FORCEINLINE rc_object_ptr_t(T *ptr) noexcept : _ptr(nullptr) {
			if (ptr) {
				_set_and_inc_ref(ptr);
			}
		}
		PBOS_FORCEINLINE rc_object_ptr_t(const this_t &other) noexcept : _ptr(nullptr) {
			if (other._ptr) {
				_set_and_inc_ref(other._ptr);
			}
		}
		PBOS_FORCEINLINE rc_object_ptr_t(this_t &&other) noexcept {
			_ptr = other._ptr;
			other._ptr = nullptr;
		}
		PBOS_FORCEINLINE ~rc_object_ptr_t() {
			reset();
		}

		PBOS_FORCEINLINE rc_object_ptr_t<T> &operator=(T *_ptr) noexcept {
			reset();
			if (_ptr) {
				_set_and_inc_ref(_ptr);
			}
			return *this;
		}
		PBOS_FORCEINLINE rc_object_ptr_t<T> &operator=(const this_t &other) noexcept {
			reset();
			_set_and_inc_ref(other._ptr);
			return *this;
		}
		PBOS_FORCEINLINE rc_object_ptr_t<T> &operator=(this_t &&other) noexcept {
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

		PBOS_FORCEINLINE bool operator<(const this_t &rhs) const noexcept {
			return _ptr < rhs._ptr;
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

	template <typename T, typename IncRef, typename DecRef>
	class custom_rc_ptr_t {
	private:
		using this_t = custom_rc_ptr_t<T, IncRef, DecRef>;

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

		PBOS_FORCEINLINE custom_rc_ptr_t() : _ptr(nullptr) {
		}
		PBOS_FORCEINLINE custom_rc_ptr_t(T *ptr) noexcept : _ptr(nullptr) {
			if (ptr) {
				_set_and_inc_ref(ptr);
			}
		}
		PBOS_FORCEINLINE custom_rc_ptr_t(const this_t &other) noexcept : _ptr(nullptr) {
			if (other._ptr) {
				_set_and_inc_ref(other._ptr);
			}
		}
		PBOS_FORCEINLINE custom_rc_ptr_t(this_t &&other) noexcept {
			_ptr = other._ptr;
			other._ptr = nullptr;
		}
		PBOS_FORCEINLINE ~custom_rc_ptr_t() {
			reset();
		}

		PBOS_FORCEINLINE custom_rc_ptr_t &operator=(T *_ptr) noexcept {
			reset();
			if (_ptr) {
				_set_and_inc_ref(_ptr);
			}
			return *this;
		}
		PBOS_FORCEINLINE custom_rc_ptr_t &operator=(const this_t &other) noexcept {
			reset();
			_set_and_inc_ref(other._ptr);
			return *this;
		}
		PBOS_FORCEINLINE custom_rc_ptr_t &operator=(this_t &&other) noexcept {
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

		PBOS_FORCEINLINE bool operator<(const this_t &rhs) const noexcept {
			return _ptr < rhs._ptr;
		}

		PBOS_FORCEINLINE bool operator>(const this_t &rhs) const noexcept {
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

		PBOS_FORCEINLINE bool operator==(const this_t &rhs) const noexcept {
			return _ptr == rhs._ptr;
		}

		PBOS_FORCEINLINE bool operator!=(const this_t &rhs) const noexcept {
			return _ptr != rhs._ptr;
		}
	};
}

#endif
