#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_FUNDAMENTAL_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_FUNDAMENTAL_

#include "is_arithmetic.hh"
#include "is_null_pointer.hh"
#include "is_void.hh"

namespace std {
	template <typename T>
	struct is_fundamental
		: std::integral_constant<
			  bool,
			  std::is_arithmetic<T>::value ||
				  std::is_void<T>::value ||
				  std::is_null_pointer<typename std::remove_cv<T>::type>::value> {};

	template <typename T>
	constexpr bool is_fundamental_v = is_fundamental<T>::value;
}

#endif
