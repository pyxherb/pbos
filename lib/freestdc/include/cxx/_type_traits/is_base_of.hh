#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_BASE_OF_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_BASE_OF_

#include "integral_constant.hh"

namespace std {
	template <typename Base, typename Derived>
	struct is_base_of : std::bool_constant<__is_base_of(Base, Derived)> {
	};

	template <typename Base, typename Derived>
	constexpr bool is_base_of_v = is_base_of<Base, Derived>::value;
}

#endif
