#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_BOUNDED_ARRAY_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_BOUNDED_ARRAY_

#include "integral_constant.hh"
#include "is_same.hh"
#include "remove_cv.hh"

namespace std {
#if __cplusplus >= 202002L
	template <class T>
	struct is_bounded_array : false_type {};

	template <class T, size_t N>
	struct is_bounded_array<T[N]> : true_type {};
#endif
}

#endif
