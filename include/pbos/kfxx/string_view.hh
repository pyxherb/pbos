#ifndef _PBOS_KFXX_STRING_VIEW_HH_
#define _PBOS_KFXX_STRING_VIEW_HH_

#include <pbos/kf/hash.h>
#include <pbos/kd/assert.h>
#include <string.h>
#include <type_traits>
#include "basedefs.hh"
#include "hash.hh"

namespace kfxx {
	class StringView {
	private:
		const char *_ptr;
		size_t _size;

	public:
		constexpr static size_t NPOS = SIZE_MAX;

		PBOS_FORCEINLINE constexpr StringView() noexcept : _ptr(nullptr), _size(0) {
		}

		PBOS_FORCEINLINE constexpr StringView(const char *s) noexcept : _ptr(s) {
			if (std::is_constant_evaluated()) {
				size_t size = 0;
				for (const char *i = s; *i; ++i, ++size);
				_size = size;
			} else {
				_size = strlen(s);
			}
		}

		PBOS_FORCEINLINE constexpr StringView(const char *s, size_t len) noexcept : _ptr(s), _size(len) {
		}

		PBOS_FORCEINLINE constexpr StringView(const StringView &other) noexcept : _ptr(other._ptr), _size(other._size) {
		}

		PBOS_FORCEINLINE constexpr StringView(StringView &&other) noexcept : _ptr(other._ptr), _size(other._size) {
			other._ptr = nullptr;
			other._size = 0;
		}

		PBOS_FORCEINLINE constexpr StringView &operator=(const StringView &other) noexcept {
			_ptr = other._ptr;
			_size = other._size;

			return *this;
		}

		PBOS_FORCEINLINE constexpr StringView &operator=(StringView &&other) noexcept {
			_ptr = other._ptr;
			_size = other._size;
			other._ptr = nullptr;
			other._size = 0;

			return *this;
		}

		~StringView() = default;

		PBOS_FORCEINLINE constexpr const char &at(size_t index) const noexcept {
			kd_dbgcheck(index < _size, "StringView out of range access error detected");

			return _ptr[index];
		}

		PBOS_FORCEINLINE constexpr const char &operator[](size_t index) const noexcept {
			return at(index);
		}

		PBOS_FORCEINLINE constexpr StringView substr(size_t index, size_t len = SIZE_MAX) const {
			kd_dbgcheck(index < _size, "StringView out of range substr error detected");

			return StringView(_ptr + index, PBOS_MIN(_size, len));
		}

		PBOS_FORCEINLINE constexpr const char *data() const {
			return _ptr;
		}

		PBOS_FORCEINLINE constexpr size_t size() const {
			return _size;
		}

		PBOS_FORCEINLINE bool operator==(const StringView &rhs) const {
			if (rhs.size() != size())
				return false;
			return !memcmp(_ptr, rhs._ptr, _size);
		}

		PBOS_FORCEINLINE bool operator>(const StringView &rhs) const {
			if (_size > rhs._size)
				return true;
			if (_size < rhs._size)
				return false;
			return memcmp(_ptr, rhs._ptr, _size) > 0;
		}

		PBOS_FORCEINLINE bool operator<(const StringView &rhs) const {
			if (_size > rhs._size)
				return false;
			if (_size < rhs._size)
				return true;
			return memcmp(_ptr, rhs._ptr, _size) < 0;
		}
	};

	template <>
	struct Hash<StringView> {
		PBOS_FORCEINLINE size_t operator()(const StringView &x) const {
			if constexpr (sizeof(size_t) == sizeof(uint32_t)) {
				return kf_djb_hash32(x.data(), x.size());
			} else {
				return kf_djb_hash64(x.data(), x.size());
			}
		}
	};

	namespace literal_suffixes {
		constexpr kfxx::StringView operator""_sv(const char *s) {
			size_t index = 0;
			while (s[index])
				++index;
			return kfxx::StringView(s, index);
		}
	}
}

#endif
