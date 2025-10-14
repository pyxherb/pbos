#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_RVALUE_REFERENCE_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_RVALUE_REFERENCE_

#include "integral_constant.hh"

namespace std {
	template <typename T>
	struct is_rvalue_reference : std::false_type {};
	template <typename T>
	struct is_rvalue_reference<T &&> : std::true_type {};
}

#endif
