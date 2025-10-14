#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_SAME_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_SAME_

#include "integral_constant.hh"

namespace std {
	template <typename T, typename U>
	struct is_same : std::false_type {};

	template <typename T>
	struct is_same<T, T> : std::true_type {};
}

#endif
