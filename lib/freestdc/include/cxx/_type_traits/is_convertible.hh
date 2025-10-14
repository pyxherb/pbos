#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_CONVERTIBLE_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_CONVERTIBLE_

#include <utility>
#include "integral_constant.hh"
#include "is_same.hh"
#include "is_void.hh"
#include "remove_cv.hh"

namespace std {
	template <typename T>
	auto _test_returnable(int) -> decltype(void(static_cast<T (*)()>(nullptr)), std::true_type{});
	template <typename>
	auto _test_returnable(...) -> std::false_type;

	template <typename From, typename To>
	auto _test_implicitly_convertible(int) -> decltype(void(std::declval<void (&)(To)>()(std::declval<From>())), std::true_type{});
	template <typename, typename>
	auto _test_implicitly_convertible(...) -> std::false_type;

	template <typename From, typename To>
	struct is_convertible : std::integral_constant<bool,
								(decltype(_test_returnable<To>(0))::value &&
									decltype(_test_implicitly_convertible<From, To>(0))::value) ||
									(std::is_void<From>::value && std::is_void<To>::value)> {};

	// TODO: Implement std::is_nothrow_convertible.
}

#endif
