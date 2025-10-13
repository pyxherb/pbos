#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_UNBOUNDED_ARRAY_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_UNBOUNDED_ARRAY_

#include "integral_constant.hh"
#include "is_same.hh"
#include "remove_cv.hh"

namespace std {
#if __cplusplus >= 202002L
	template <class T>
	struct is_unbounded_array : std::false_type {};

	template <class T>
	struct is_unbounded_array<T[]> : std::true_type {};
#endif
}

#endif
