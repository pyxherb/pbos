#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_FLOATING_POINT_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_FLOATING_POINT_

#include "integral_constant.hh"
#include "is_same.hh"
#include "remove_cv.hh"

namespace std {
	template <typename T>
	struct is_floating_point
		: std::integral_constant<
			  bool,
			  // Note: standard floating-point types
			  std::is_same<float, typename remove_cv<T>::type>::value ||
				  std::is_same<double, typename remove_cv<T>::type>::value ||
				  std::is_same<long double, typename remove_cv<T>::type>::value> {};
}

#endif
