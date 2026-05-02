#ifndef _PBOS_KFXX_STRING_VIEW_HH_
#define _PBOS_KFXX_STRING_VIEW_HH_

#include <pbos/kf/hash.h>
#include <pbos/km/assert.h>
#include <string.h>
#include <type_traits>
#include "basedefs.hh"
#include "hash.hh"

namespace kfxx {
	class string_view {
	private:
		const char *_ptr;
		size_t _size;

	public:
		constexpr static size_t NPOS = SIZE_MAX;

		PBOS_FORCEINLINE constexpr string_view() noexcept : _ptr(nullptr), _size(0) {
		}

		PBOS_FORCEINLINE constexpr string_view(const char *s) noexcept : _ptr(s) {
			if (std::is_constant_evaluated()) {
				size_t size = 0;
				for (const char *i = s; *i; ++i, ++size);
				_size = size;
			} else {
				_size = strlen(s);
			}
		}

		PBOS_FORCEINLINE constexpr string_view(const char *s, size_t len) noexcept : _ptr(s), _size(len) {
		}

		PBOS_FORCEINLINE constexpr string_view(const string_view &other) noexcept : _ptr(other._ptr), _size(other._size) {
		}

		PBOS_FORCEINLINE constexpr string_view(string_view &&other) noexcept : _ptr(other._ptr), _size(other._size) {
			other._ptr = nullptr;
			other._size = 0;
		}

		PBOS_FORCEINLINE constexpr string_view &operator=(const string_view &other) noexcept {
			_ptr = other._ptr;
			_size = other._size;

			return *this;
		}

		PBOS_FORCEINLINE constexpr string_view &operator=(string_view &&other) noexcept {
			_ptr = other._ptr;
			_size = other._size;
			other._ptr = nullptr;
			other._size = 0;

			return *this;
		}

		~string_view() = default;

		PBOS_FORCEINLINE constexpr const char &at(size_t index) const noexcept {
			kd_dbgcheck(index < _size, "string_view out of range access error detected");

			return _ptr[index];
		}

		PBOS_FORCEINLINE constexpr const char &operator[](size_t index) const noexcept {
			return at(index);
		}

		PBOS_FORCEINLINE constexpr string_view substr(size_t index, size_t len = SIZE_MAX) const {
			kd_dbgcheck(index < _size, "string_view out of range substr error detected");

			return string_view(_ptr + index, PBOS_MIN(_size, len));
		}

		PBOS_FORCEINLINE constexpr const char *data() const {
			return _ptr;
		}

		PBOS_FORCEINLINE constexpr size_t size() const {
			return _size;
		}

		PBOS_FORCEINLINE bool operator==(const string_view &rhs) const {
			if (rhs.size() != size())
				return false;
			return !memcmp(_ptr, rhs._ptr, _size);
		}

		PBOS_FORCEINLINE bool operator>(const string_view &rhs) const {
			if (_size > rhs._size)
				return true;
			if (_size < rhs._size)
				return false;
			return memcmp(_ptr, rhs._ptr, _size) > 0;
		}

		PBOS_FORCEINLINE bool operator<(const string_view &rhs) const {
			if (_size > rhs._size)
				return false;
			if (_size < rhs._size)
				return true;
			return memcmp(_ptr, rhs._ptr, _size) < 0;
		}
	};

	template <>
	struct hash<string_view> {
		PBOS_FORCEINLINE size_t operator()(const string_view &x) const {
			if constexpr (sizeof(size_t) == sizeof(uint32_t)) {
				return kf_djb_hash32(x.data(), x.size());
			} else {
				return kf_djb_hash64(x.data(), x.size());
			}
		}
	};

	namespace literal_suffixes {
		constexpr kfxx::string_view operator""_sv(const char *s) {
			size_t index = 0;
			while (s[index])
				++index;
			return kfxx::string_view(s, index);
		}
	}
}

#endif
