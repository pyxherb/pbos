#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_UNSIGNED_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_UNSIGNED_

#include "is_arithmetic.hh"

namespace std {
	template <typename T, bool = std::is_arithmetic<T>::value>
	struct _is_unsigned_impl : std::integral_constant<bool, T(0) < T(-1)> {};

	template <typename T>
	struct _is_unsigned_impl<T, false> : std::false_type {};

	template <typename T>
	struct is_unsigned : _is_unsigned_impl<T>::type {};
}

#endif
