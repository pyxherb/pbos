#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_UNION_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_UNION_

#include "integral_constant.hh"

namespace std {
	template <typename T>
	struct is_union : std::bool_constant<__is_union(T)> {
	};

	template <typename T>
	constexpr bool is_union_v = is_union<T>::value;
}

#endif
