#ifndef _PBOS_FS_FILE_HH_
#define _PBOS_FS_FILE_HH_

#include "file.h"

namespace fs {
	class fnode_ptr_t {
	private:
		using this_t = fnode_ptr_t;

		fs_fnode_t *_ptr = nullptr;

		PBOS_FORCEINLINE void _set_and_inc_ref(fs_fnode_t *_ptr) {
			fs_inc_fnode_ref(_ptr);
			this->_ptr = _ptr;
		}

	public:
		PBOS_FORCEINLINE void reset() noexcept {
			if (_ptr)
				fs_dec_fnode_ref(_ptr);
			_ptr = nullptr;
		}

		PBOS_FORCEINLINE fnode_ptr_t() : _ptr(nullptr) {
		}
		PBOS_FORCEINLINE fnode_ptr_t(fs_fnode_t *ptr) noexcept : _ptr(nullptr) {
			if (ptr) {
				_set_and_inc_ref(ptr);
			}
		}
		PBOS_FORCEINLINE fnode_ptr_t(const this_t &other) noexcept : _ptr(nullptr) {
			if (other._ptr) {
				_set_and_inc_ref(other._ptr);
			}
		}
		PBOS_FORCEINLINE fnode_ptr_t(this_t &&other) noexcept {
			_ptr = other._ptr;
			other._ptr = nullptr;
		}
		PBOS_FORCEINLINE ~fnode_ptr_t() {
			reset();
		}

		PBOS_FORCEINLINE fnode_ptr_t &operator=(fs_fnode_t *_ptr) noexcept {
			reset();
			if (_ptr) {
				_set_and_inc_ref(_ptr);
			}
			return *this;
		}
		PBOS_FORCEINLINE fnode_ptr_t &operator=(const this_t &other) noexcept {
			reset();
			_set_and_inc_ref(other._ptr);
			return *this;
		}
		PBOS_FORCEINLINE fnode_ptr_t &operator=(this_t &&other) noexcept {
			reset();

			_ptr = other._ptr;

			other._ptr = nullptr;

			return *this;
		}

		PBOS_FORCEINLINE fs_fnode_t *get() const noexcept {
			return _ptr;
		}
		PBOS_FORCEINLINE fs_fnode_t *&get_ref() noexcept {
			reset();
			return _ptr;
		}
		PBOS_FORCEINLINE fs_fnode_t *&get_ref_without_release() noexcept {
			return _ptr;
		}
		PBOS_FORCEINLINE fs_fnode_t *const &get_Ref_without_release() const noexcept {
			return _ptr;
		}
		PBOS_FORCEINLINE fs_fnode_t **get_addr() noexcept {
			reset();
			return &_ptr;
		}
		PBOS_FORCEINLINE fs_fnode_t **get_addr_without_release() noexcept {
			return &_ptr;
		}
		PBOS_FORCEINLINE fs_fnode_t *const *get_addr_without_release() const noexcept {
			return &_ptr;
		}
		PBOS_FORCEINLINE fs_fnode_t *release() noexcept {
			fs_fnode_t *ptr = _ptr;
			_ptr = nullptr;
			return ptr;
		}
		PBOS_FORCEINLINE fs_fnode_t *operator->() const noexcept {
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
}

#endif
