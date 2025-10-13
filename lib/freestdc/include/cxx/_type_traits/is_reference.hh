#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_REFERENCE_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_REFERENCE_

#include "integral_constant.hh"

namespace std {
	template <class T>
	struct is_reference : std::false_type {};
	template <class T>
	struct is_reference<T &> : std::true_type {};
	template <class T>
	struct is_reference<T &&> : std::true_type {};
}

#endif
