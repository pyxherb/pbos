#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_UNION_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_UNION_

#include "integral_constant.hh"

namespace std {
	template <typename T>
	struct is_union : std::bool_constant<__is_union(T)> {
	};
}

#endif
