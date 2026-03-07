#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_CONVERTIBLE_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_CONVERTIBLE_

#include <utility>
#include "integral_constant.hh"
#include "is_same.hh"
#include "is_void.hh"
#include "remove_cv.hh"
#include "conditional.hh"

namespace std {
	template <typename T>
	auto _test_returnable(int) -> decltype(void(static_cast<T (*)()>(nullptr)), true_type{});
	template <typename>
	auto _test_returnable(...) -> false_type;

	template <typename From, typename To>
	auto _test_implicitly_convertible(int) -> decltype(void(declval<void (&)(To)>()(declval<From>())), true_type{});
	template <typename, typename>
	auto _test_implicitly_convertible(...) -> false_type;

	template <typename From, typename To>
	struct is_convertible : integral_constant<bool,
								(decltype(_test_returnable<To>(0))::value &&
									decltype(_test_implicitly_convertible<From, To>(0))::value) ||
									(is_void<From>::value && is_void<To>::value)> {};

	template <typename From, typename To>
	constexpr bool is_convertible_v = is_convertible<From, To>::value;

	// TODO: Test if the nothrow version works.
	template <typename T>
	auto _test_nothrow_returnable(int) -> typename conditional<decltype(_test_returnable<T>())::value && noexcept(void(static_cast<T (*)()>(nullptr))), true_type, false_type>::type;
	template <typename T>
	auto _test_nothrow_returnable(...) -> false_type;

	template <typename From, typename To>
	auto _test_nothrow_implicitly_convertible(int) -> typename conditional<decltype(_test_implicitly_convertible<From, To>())::value && noexcept(void(declval<void (&)(To)>()(declval<From>()))), true_type, false_type>::type;
	template <typename, typename>
	auto _test_nothrow_implicitly_convertible(...) -> false_type;

	template <typename From, typename To>
	struct is_nothrow_convertible : integral_constant<bool,
								(decltype(_test_nothrow_returnable<To>(0))::value &&
									decltype(_test_nothrow_implicitly_convertible<From, To>(0))::value) ||
									(is_void<From>::value && is_void<To>::value)> {};

	template <typename From, typename To>
	constexpr bool is_nothrow_convertible_v = is_nothrow_convertible<From, To>::value;
}

#endif
