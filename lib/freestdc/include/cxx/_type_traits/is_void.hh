#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_VOID_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_VOID_

#include "is_same.hh"
#include "remove_cv.hh"

namespace std {
	template <class T>
	struct is_void : std::is_same<void, typename std::remove_cv<T>::type> {};
}

#endif
