#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_ENUM_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_ENUM_

#include "integral_constant.hh"

namespace std {
	template <typename T>
	struct is_enum : integral_constant<bool, __is_enum(T)> {
	};

	template <typename T>
	constexpr bool is_enum_v = is_enum<T>::value;
}

#endif
