#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_REFERENCE_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_REFERENCE_

#include "integral_constant.hh"

namespace std {
	template <typename T>
	struct is_reference : std::false_type {};
	template <typename T>
	struct is_reference<T &> : std::true_type {};
	template <typename T>
	struct is_reference<T &&> : std::true_type {};

	template <typename T>
	constexpr bool is_reference_v = is_reference<T>::value;
}

#endif
