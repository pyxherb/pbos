#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_VOID_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_VOID_

#include "is_same.hh"
#include "remove_cv.hh"

namespace std {
	template <typename T>
	struct is_void : std::is_same<void, typename std::remove_cv<T>::type> {};

	template <typename T>
	constexpr bool is_void_v = is_void<T>::value;
}

#endif
