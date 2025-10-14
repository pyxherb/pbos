#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_SCALAR_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_SCALAR_

#include "is_arithmetic.hh"
#include "is_enum.hh"
#include "is_member_pointer.hh"
#include "is_null_pointer.hh"
#include "is_pointer.hh"

namespace std {
	template <typename T>
	struct is_scalar : std::integral_constant<
						   bool,
						   std::is_arithmetic<T>::value ||
							   std::is_enum<T>::value ||
							   std::is_pointer<T>::value ||
							   std::is_member_pointer<T>::value ||
							   std::is_null_pointer<T>::value> {};
}

#endif
