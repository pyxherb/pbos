#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_STANDARD_LAYOUT_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_STANDARD_LAYOUT_

#include "integral_constant.hh"

namespace std {
	template <typename T>
	struct is_standard_layout : std::bool_constant<__is_standard_layout(T)> {};
}

#endif
