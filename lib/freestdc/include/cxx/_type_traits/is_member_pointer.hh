#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_MEMBER_POINTER_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_MEMBER_POINTER_

#include "integral_constant.hh"
#include "remove_cv.hh"

namespace std {
	template <typename T>
	struct _is_member_pointer_impl : std::false_type {};

	template <typename T, typename U>
	struct _is_member_pointer_impl<T U::*> : std::true_type {};

	template <typename T>
	struct is_member_pointer : _is_member_pointer_impl<typename std::remove_cv<T>::type> {};
}

#endif
