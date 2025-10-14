#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_ARITHMETIC_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_ARITHMETIC_

#include "integral_constant.hh"
#include "is_floating_point.hh"
#include "is_integral.hh"

namespace std {
	template <typename T>
	struct is_arithmetic : std::integral_constant<bool,
							   std::is_integral<T>::value ||
								   std::is_floating_point<T>::value> {};
	template <typename T>
	constexpr bool is_arithmetic_v = is_arithmetic<T>::value;
}

#endif
