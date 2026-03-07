#ifndef _FREESTDC_CXX_COROUTINE_COROUTINE_HANDLE_
#define _FREESTDC_CXX_COROUTINE_COROUTINE_HANDLE_

#include <functional>
#include <memory>
#include <type_traits>

namespace std {
	template <typename Promise = void>
	struct coroutine_handle;

	template <>
	struct coroutine_handle<void> {
	private:
		void *_handle = nullptr;

	public:
		constexpr coroutine_handle() noexcept = default;

		constexpr coroutine_handle(nullptr_t) noexcept {}
		constexpr coroutine_handle &operator=(nullptr_t) noexcept {
			_handle = nullptr;
			return *this;
		}

		constexpr void *address() const noexcept {
			return _handle;
		}

		constexpr static coroutine_handle from_address(void *address) {
			coroutine_handle h;
			h._handle = address;
			return h;
		}

		constexpr explicit operator bool() { return _handle; }

		inline bool done() const {
			return __builtin_coro_done(_handle);
		}

		inline void resume() const {
			__builtin_coro_resume(_handle);
		}

		inline void destroy() const {
			__builtin_coro_destroy(_handle);
		}

		inline void operator()() {
			resume();
		}
	};

	template <typename Promise>
	struct coroutine_handle {
	private:
		void *_handle = nullptr;

	public:
		constexpr coroutine_handle() noexcept = default;

		constexpr coroutine_handle(nullptr_t) noexcept {}
		constexpr coroutine_handle &operator=(nullptr_t) noexcept {
			_handle = nullptr;
			return *this;
		}

		static coroutine_handle from_promise(Promise &promise) {
			coroutine_handle h;
			h._handle = __builtin_coro_promise(
				addressof(
					const_cast<remove_cv<Promise> &>(promise),
					alignof(Promise),
					true));
		}

		constexpr void *address() const noexcept {
			return _handle;
		}

		constexpr static coroutine_handle from_address(void *address) {
			coroutine_handle h;
			h._handle = address;
			return h;
		}

		constexpr operator coroutine_handle<>() const noexcept {
			return coroutine_handle<>::from_address(address());
		}

		constexpr explicit operator bool() { return _handle; }

		inline bool done() const {
			return __builtin_coro_done(_handle);
		}

		inline void resume() const {
			__builtin_coro_resume(_handle);
		}

		inline void destroy() const {
			__builtin_coro_destroy(_handle);
		}

		inline void operator()() {
			resume();
		}

		Promise &promise() const {
			return *static_cast<Promise *>(__builtin_coro_promise(_handle, alignof(Promise), false));
		}
	};

	constexpr inline bool operator==(coroutine_handle<> lhs, coroutine_handle<> rhs) {
		return lhs.address() == rhs.address();
	}

	constexpr inline bool operator<=>(coroutine_handle<> lhs, coroutine_handle<> rhs) {
		return compare_three_way{}(lhs.address(), rhs.address());
	}

	// TODO: Implement the hasher.
}

#endif
