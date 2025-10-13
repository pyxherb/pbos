#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_ABSTRACT_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_ABSTRACT_

#include "integral_constant.hh"

namespace std {
	template <class T>
	struct is_abstract : std::bool_constant<__is_abstract (T)> {};
}

#endif
