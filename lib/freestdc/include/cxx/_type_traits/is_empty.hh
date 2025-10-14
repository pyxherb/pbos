#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_EMPTY_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_EMPTY_

#include "integral_constant.hh"

namespace std {
	template <typename T>
	struct is_empty : std::bool_constant<__is_empty(T)> {};
}

#endif
