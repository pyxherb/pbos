#ifndef _PBOS_KFXX_SPAN_H_
#define _PBOS_KFXX_SPAN_H_

#include <pbos/kd/assert.h>
#include <type_traits>
#include "basedefs.hh"

namespace peff {
	template <typename T>
	class Span {
	private:
		T *_ptr;
		size_t _length;

	public:
		Span() : _ptr(nullptr), _length(0) {}
		Span(T *ptr, size_t length) : _ptr(ptr), _length(length) {}
		template <size_t N>
		Span(T array[N]) : _ptr(array), _length(N) {}
		~Span() = default;
		Span(const Span<T> &) = default;
		Span(Span<T> &&) = default;

		PBOS_FORCEINLINE T *data() const noexcept {
			return _ptr;
		}

		PBOS_FORCEINLINE T &at(size_t index) const noexcept {
			return _ptr + index;
		}

		PBOS_FORCEINLINE T &operator[](size_t index) const noexcept {
			return _ptr + index;
		}

		PBOS_FORCEINLINE size_t size() const noexcept {
			return _length;
		}

		PBOS_FORCEINLINE bool empty() const noexcept {
			return !_length;
		}

		PBOS_FORCEINLINE T &front() const noexcept {
			kd_dbgcheck(_length, "The span is empty");
			return _ptr;
		}

		PBOS_FORCEINLINE T &back() const noexcept {
			kd_dbgcheck(_length, "The span is empty");
			return _ptr + _length - 1;
		}

		PBOS_FORCEINLINE Span<T> subspan(size_t index, size_t length) const noexcept {
			kd_dbgcheck(index + length <= _length, "Out of span range");
			return { _ptr + index, length };
		}
	};
}

#endif
