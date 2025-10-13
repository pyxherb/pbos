#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_VOLATILE_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_VOLATILE_

#include "integral_constant.hh"

namespace std {
	template <class T>
	struct is_volatile : std::false_type {};
	template <class T>
	struct is_volatile<volatile T> : std::true_type {};
}

#endif
