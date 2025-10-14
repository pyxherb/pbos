#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_CONST_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_CONST_

#include "integral_constant.hh"

namespace std {
	template <typename T>
	struct is_const : std::false_type {};
	template <typename T>
	struct is_const<const T> : std::true_type {};
}

#endif
