#ifndef _FREESTDC_CXX_CONCEPTS_ARITHM_TRAITS_
#define _FREESTDC_CXX_CONCEPTS_ARITHM_TRAITS_

#include <type_traits>

namespace std {
	template <class T>
	concept integral = std::is_integral_v<T>;

	template <class T>
	concept signed_integral = std::integral<T> && std::is_signed_v<T>;

	template <class T>
	concept unsigned_integral = std::integral<T> && !std::signed_integral<T>;

	template <class T>
	concept floating_point = std::is_floating_point_v<T>;
}

#endif
