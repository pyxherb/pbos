#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_NULL_POINTER_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_NULL_POINTER_

#include <cstddef>
#include "is_same.hh"
#include "remove_cv.hh"

namespace std {
	template <typename T>
	struct is_null_pointer : is_same<std::nullptr_t, typename remove_cv<T>::type> {};
}

#endif
