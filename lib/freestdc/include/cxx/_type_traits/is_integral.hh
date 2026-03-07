#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_INTEGRAL_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_INTEGRAL_

#include "integral_constant.hh"
#include "is_same.hh"
#include "remove_cv.hh"

namespace std {
	template <typename T>
	struct is_integral
		: std::integral_constant<
			  bool,
#if __cplusplus >= 202002L
			  std::is_same<char8_t, typename remove_cv<T>::type>::value ||
#endif
				  std::is_same<char16_t, typename remove_cv<T>::type>::value ||
				  std::is_same<char32_t, typename remove_cv<T>::type>::value ||
				  std::is_same<wchar_t, typename remove_cv<T>::type>::value ||
				  std::is_same<bool, typename remove_cv<T>::type>::value ||
				  std::is_same<char, typename remove_cv<T>::type>::value ||
				  std::is_same<short, typename remove_cv<T>::type>::value ||
				  std::is_same<int, typename remove_cv<T>::type>::value ||
				  std::is_same<long, typename remove_cv<T>::type>::value ||
				  std::is_same<long long, typename remove_cv<T>::type>::value ||
				  std::is_same<unsigned char, typename remove_cv<T>::type>::value ||
				  std::is_same<unsigned short, typename remove_cv<T>::type>::value ||
				  std::is_same<unsigned int, typename remove_cv<T>::type>::value ||
				  std::is_same<unsigned long, typename remove_cv<T>::type>::value ||
				  std::is_same<unsigned long long, typename remove_cv<T>::type>::value> {
	};

	template <typename T>
	constexpr bool is_integral_v = is_integral<T>::value;
}

#endif
