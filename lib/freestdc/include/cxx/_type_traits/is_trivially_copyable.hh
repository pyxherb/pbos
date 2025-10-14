#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_TRIVIALLY_COPYABLE_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_TRIVIALLY_COPYABLE_

#include "integral_constant.hh"

namespace std {
	template <typename T>
	struct is_trivially_copyable : std::bool_constant<__is_trivially_copyable(T)> {};
}

#endif
