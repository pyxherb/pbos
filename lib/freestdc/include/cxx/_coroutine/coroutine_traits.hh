#ifndef _FREESTDC_CXX_COROUTINE_COROUTINE_TRAITS_
#define _FREESTDC_CXX_COROUTINE_COROUTINE_TRAITS_

#include <type_traits>

namespace std {
	template <typename T, typename = void, typename... Args>
	struct coroutine_traits {};

	template <typename T, typename... Args>
	struct coroutine_traits<T, std::void_t<typename T::promise_type>, Args...> {
		using promise_type = typename T::promise_type;
	};
}

#endif
