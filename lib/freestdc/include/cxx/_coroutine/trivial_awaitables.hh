#ifndef _FREESTDC_CXX_COROUTINE_TRIVIAL_AWAITABLES_
#define _FREESTDC_CXX_COROUTINE_TRIVIAL_AWAITABLES_

#include "coroutine_handle.hh"

namespace std {
	struct suspend_never {
		constexpr bool await_ready() const noexcept { return true; }
		constexpr void await_suspend(coroutine_handle<>) const noexcept {}
		constexpr void await_resume() const noexcept {}
	};

	struct suspend_always {
		constexpr bool await_ready() const noexcept { return false; }
		constexpr void await_suspend(coroutine_handle<>) const noexcept {}
		constexpr void await_resume() const noexcept {}
	};
}

#endif
