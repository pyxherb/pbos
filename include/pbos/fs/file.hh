#ifndef _PBOS_FS_FILE_HH_
#define _PBOS_FS_FILE_HH_

#include <pbos/kfxx/rcobj.hh>
#include <pbos/kfxx/unique_ptr.hh>
#include "file.h"

namespace fs {
	struct fnode_inc_ref {
		PBOS_FORCEINLINE void operator()(fs_fnode_t *ptr) noexcept {
			fs_ref_fnode(ptr);
		}
	};

	struct fnode_dec_ref {
		PBOS_FORCEINLINE void operator()(fs_fnode_t *ptr) noexcept {
			fs_unref_fnode(ptr);
		}
	};

	using fnode_ptr = kfxx::custom_rc_ptr<fs_fnode_t, fnode_inc_ref, fnode_dec_ref>;

	PBOS_FORCEINLINE void fcb_ptr_null_hook(fs_fcb_t *, km_result_t) noexcept {
	}

	template <typename H>
	struct fcb_ptr {
	private:
		static_assert(std::is_invocable_v<H, fs_fcb_t *, km_result_t>, "The fail hook is not invocable");
		fs_fcb_t *_ptr;
		H _fail_hook;

		using ThisType = fs::fcb_ptr<H>;

	public:
		PBOS_FORCEINLINE void reset() noexcept {
			if (_ptr) {
				km_result_t result = fs_close(_ptr);
				if (KM_FAILED(result))
					_fail_hook(_ptr, result);
				_ptr = nullptr;
			}
		}

		PBOS_FORCEINLINE fcb_ptr(H &&fail_hook) noexcept : _ptr(nullptr), _fail_hook(fail_hook) {
		}
		PBOS_FORCEINLINE fcb_ptr(fs_fcb_t *ptr, H &&fail_hook) noexcept : _ptr(ptr), _fail_hook(fail_hook) {
		}
		fcb_ptr(const ThisType &other) noexcept = delete;
		PBOS_FORCEINLINE fcb_ptr(ThisType &&other) noexcept : _ptr(other._ptr), _fail_hook(other._fail_hook) {
			other._ptr = nullptr;
		}
		PBOS_FORCEINLINE ~fcb_ptr() {
			reset();
		}

		PBOS_FORCEINLINE ThisType &operator=(fs_fcb_t *ptr) noexcept {
			reset();
			_ptr = ptr;
			return *this;
		}
		ThisType &operator=(const ThisType &other) = delete;
		PBOS_FORCEINLINE ThisType &operator=(ThisType &&other) noexcept {
			reset();

			_fail_hook = std::move(other._fail_hook);
			_ptr = other._ptr;
			other._ptr = nullptr;

			return *this;
		}

		PBOS_FORCEINLINE fs_fcb_t *get() const noexcept {
			return _ptr;
		}
		PBOS_FORCEINLINE fs_fcb_t *&get_ref() noexcept {
			reset();
			return _ptr;
		}
		PBOS_FORCEINLINE fs_fcb_t *&get_ref_without_release() noexcept {
			return _ptr;
		}
		PBOS_FORCEINLINE fs_fcb_t *const &get_Ref_without_release() const noexcept {
			return _ptr;
		}
		PBOS_FORCEINLINE fs_fcb_t **get_addr() noexcept {
			reset();
			return &_ptr;
		}
		PBOS_FORCEINLINE fs_fcb_t **get_addr_without_release() noexcept {
			return &_ptr;
		}
		PBOS_FORCEINLINE fs_fcb_t *const *get_addr_without_release() const noexcept {
			return &_ptr;
		}
		PBOS_FORCEINLINE fs_fcb_t **operator&() noexcept {
			reset();
			return &_ptr;
		}
		PBOS_FORCEINLINE fs_fcb_t *release() noexcept {
			fs_fcb_t *ptr = _ptr;
			_ptr = nullptr;
			return ptr;
		}
		PBOS_FORCEINLINE fs_fcb_t *operator->() const noexcept {
			return _ptr;
		}

		PBOS_FORCEINLINE bool operator<(const fs_fcb_t *rhs) const noexcept {
			return _ptr < rhs;
		}

		PBOS_FORCEINLINE bool operator>(const fs_fcb_t *rhs) const noexcept {
			return _ptr > rhs;
		}

		PBOS_FORCEINLINE bool operator==(const fs_fcb_t *rhs) const noexcept {
			return _ptr == rhs;
		}

		PBOS_FORCEINLINE bool operator!=(const fs_fcb_t *rhs) const noexcept {
			return _ptr != rhs;
		}

		PBOS_FORCEINLINE bool operator<(const ThisType &rhs) const noexcept {
			return _ptr < rhs._ptr;
		}

		PBOS_FORCEINLINE bool operator>(const ThisType &rhs) const noexcept {
			return _ptr > rhs._ptr;
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
