#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_MEMBER_OBJECT_POINTER_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_MEMBER_OBJECT_POINTER_

#include "is_member_function_pointer.hh"
#include "is_member_pointer.hh"

namespace std {
	template <class T>
	struct is_member_object_pointer : std::integral_constant<
										  bool,
										  std::is_member_pointer<T>::value &&
											  !std::is_member_function_pointer<T>::value> {};

	template <class T>
	constexpr bool is_member_object_pointer_v = is_member_object_pointer<T>::value;
}

#endif
