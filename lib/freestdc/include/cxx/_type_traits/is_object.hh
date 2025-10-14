#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_OBJECT_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_OBJECT_

#include "is_scalar.hh"
#include "is_array.hh"
#include "is_union.hh"
#include "is_class.hh"

namespace std {
	template <typename T>
	struct is_object : std::integral_constant<bool,
						   is_scalar<T>::value ||
							   is_array<T>::value ||
							   is_union<T>::value ||
							   is_class<T>::value> {};
}

#endif
