#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_POLYMORPHIC_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_POLYMORPHIC_

#include "integral_constant.hh"

namespace std {
	template <typename T>
	struct is_polymorphic : std::bool_constant<__is_polymorphic(T)> {};

	template <typename T>
	constexpr bool is_polymorphic_v = is_polymorphic<T>::value;
}

#endif
