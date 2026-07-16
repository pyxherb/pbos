#ifndef _PBOS_KM_STRING_HH_
#define _PBOS_KM_STRING_HH_

#include <pbos/kfxx/string_view.hh>
#include "string.h"

namespace km {
	struct shared_string_ref {
	private:
		km_shared_string_handle_t _handle;

		PBOS_FORCEINLINE void _reset() {
			if (_handle) {
				km_unref_shared_string(_handle);
				_handle = nullptr;
			}
		}

	public:
		PBOS_FORCEINLINE shared_string_ref() noexcept : _handle(nullptr) {
		}
		PBOS_FORCEINLINE shared_string_ref(km_shared_string_handle_t handle) noexcept : _handle(handle) {
			km_ref_shared_string(_handle);
		}
		PBOS_FORCEINLINE shared_string_ref(const shared_string_ref &rhs) noexcept : _handle(rhs._handle) {
			km_ref_shared_string(_handle);
		}
		PBOS_FORCEINLINE shared_string_ref(shared_string_ref &&rhs) noexcept : _handle(rhs._handle) {
			rhs._handle = nullptr;
		}

		PBOS_FORCEINLINE shared_string_ref &operator=(const shared_string_ref &rhs) noexcept {
			_reset();
			if (rhs._handle) {
				_handle = rhs._handle;
				km_ref_shared_string(_handle);
			}
			return *this;
		}
		PBOS_FORCEINLINE shared_string_ref &operator=(shared_string_ref &&rhs) noexcept {
			_reset();
			if (rhs._handle) {
				_handle = rhs._handle;
				rhs._handle = nullptr;
			}
			return *this;
		}

		PBOS_FORCEINLINE km_shared_string_handle_t *get_addr() noexcept {
			_reset();
			return &_handle;
		}

		PBOS_FORCEINLINE km_shared_string_handle_t *get_addr_without_release() noexcept {
			return &_handle;
		}

		PBOS_FORCEINLINE km_shared_string_handle_t release() noexcept {
			km_shared_string_handle_t handle = _handle;
			_handle = nullptr;
			return handle;
		}

		PBOS_FORCEINLINE kfxx::string_view get() const noexcept {
			size_t length;
			const char *s = km_lookup_shared_string(_handle, &length);
			return kfxx::string_view(s, length);
		}

		PBOS_FORCEINLINE operator kfxx::string_view() const noexcept {
			return get();
		}

		PBOS_FORCEINLINE std::strong_ordering operator<=>(const shared_string_ref &rhs) const noexcept {
			return _handle <=> rhs._handle;
		}

		bool operator<(const shared_string_ref &) const noexcept = default;
		bool operator>(const shared_string_ref &) const noexcept = default;
		bool operator==(const shared_string_ref &) const noexcept = default;
	};
}

#endif
