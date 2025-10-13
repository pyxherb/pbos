#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_ARRAY_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_ARRAY_

#include "integral_constant.hh"
#include <cstddef>

namespace std {
	template <class T>
	struct is_array : std::false_type {};

	template <class T>
	struct is_array<T[]> : std::true_type {};

	template <class T, size_t N>
	struct is_array<T[N]> : std::true_type {};
}

#endif
