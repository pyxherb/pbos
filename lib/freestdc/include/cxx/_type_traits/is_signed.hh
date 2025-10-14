#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_SIGNED_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_SIGNED_

#include "is_arithmetic.hh"

namespace std {
	template <typename T, bool = std::is_arithmetic<T>::value>
	struct _is_signed_impl : std::integral_constant<bool, T(-1) < T(0)> {};

	template <typename T>
	struct _is_signed_impl<T, false> : std::false_type {};

	template <typename T>
	struct is_signed : _is_signed_impl<T>::type {};

	template <typename T>
	constexpr bool is_signed_v = is_signed<T>::value;
}

#endif
