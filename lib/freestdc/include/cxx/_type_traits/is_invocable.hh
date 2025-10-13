#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_INVOCABLE_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_INVOCABLE_

#include <functional>
#include "integral_constant.hh"
#include "is_same.hh"
#include "remove_cv.hh"
#include "void_t.hh"

namespace std {
	template <typename T, typename = void>
	struct _is_invocable_impl : std::false_type {};

	template <typename F, typename... Args>
	struct _is_invocable_impl<
		F(Args...),
		void_t<decltype(std::declval<F>()(std::declval<Args>()...))>> : std::true_type {};

	template <typename T, typename = void>
	struct _is_nothrow_invocable_impl : std::false_type {};

	template <typename F, typename... Args>
	struct _is_nothrow_invocable_impl<
		F(Args...),
		void_t<decltype(std::declval<F>()(std::declval<Args>()...))>> : std::bool_constant<noexcept(std::declval<F>()(std::declval<Args>()...))> {};

	template <typename F, typename... Args>
	struct is_invocable : _is_invocable_impl<F(Args...)> {};

	template <typename F, typename... Args>
	struct is_nothrow_invocable : _is_nothrow_invocable_impl<F(Args...)> {};

	template <typename F, typename... Args>
	inline constexpr bool is_invocable_v = is_invocable<F, Args...>::value;

	template <typename F, typename... Args>
	inline constexpr bool is_nothrow_invocable_v = is_nothrow_invocable<F, Args...>::value;

	template <typename F, typename... ArgTypes>
	struct invoke_result {
		using type = decltype(invoke(std::declval<F>(), std::declval<ArgTypes>()...));
	};

	template <typename F, typename... ArgTypes>
	using invoke_result_t = invoke_result<F, ArgTypes...>;
}

#endif
