#ifndef _FREESTDC_CXX_TYPE_TRAITS_HAS_UNIQUE_OBJECT_REPRESENTATIONS_
#define _FREESTDC_CXX_TYPE_TRAITS_HAS_UNIQUE_OBJECT_REPRESENTATIONS_

#include "integral_constant.hh"

namespace std {
#if defined(__GNUC__) || defined(__clang__)
	template <typename T>
	struct has_unique_object_representations : std::bool_constant<__has_unique_object_representations(T)> {};

	template <typename T>
	constexpr bool has_unique_object_representations_v =
		has_unique_object_representations<T>::value;

	#define __cpp_lib_has_unique_object_representations 201606L
#endif
}

#endif
