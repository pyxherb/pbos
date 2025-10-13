#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_POINTER_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_POINTER_

#include "integral_constant.hh"

namespace std {
	template <class T>
	struct is_pointer : std::false_type {};

	template <class T>
	struct is_pointer<T *> : std::true_type {};

	template <class T>
	struct is_pointer<T *const> : std::true_type {};

	template <class T>
	struct is_pointer<T *volatile> : std::true_type {};

	template <class T>
	struct is_pointer<T *const volatile> : std::true_type {};
}

#endif
