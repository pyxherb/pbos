#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_COMPOUND_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_COMPOUND_

#include "is_fundamental.hh"

namespace std {
	template <typename T>
	struct is_compound : std::integral_constant<bool, !std::is_fundamental<T>::value> {};

	template <typename T>
	constexpr bool is_compound_v = is_compound<T>::value;
}

#endif
