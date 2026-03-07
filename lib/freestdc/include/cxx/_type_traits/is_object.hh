#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_OBJECT_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_OBJECT_

#include "is_array.hh"
#include "is_class.hh"
#include "is_scalar.hh"
#include "is_union.hh"

namespace std {
	template <typename T>
	struct is_object : std::integral_constant<bool,
						   is_scalar<T>::value ||
							   is_array<T>::value ||
							   is_union<T>::value ||
							   is_class<T>::value> {};

	template <typename T>
	constexpr bool is_object_v = is_object<T>::value;
}

#endif
