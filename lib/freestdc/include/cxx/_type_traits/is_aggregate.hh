#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_AGGREGATE_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_AGGREGATE_

#include "integral_constant.hh"

namespace std {
	template <typename T>
	struct is_aggregate : std::bool_constant<__is_aggregate(T)> {
	};

	template <typename T>
	constexpr bool is_aggregate_v = is_aggregate<T>::value;

#define __cpp_lib_is_aggregate 201703L
}

#endif
