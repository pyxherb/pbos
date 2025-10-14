#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_CLASS_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_CLASS_

#include "integral_constant.hh"

namespace std {
	template <typename T>
	struct is_class : std::bool_constant<__is_class(T)> {
	};

	template <typename T>
	constexpr bool is_class_v = is_class<T>::value;
}

#endif
