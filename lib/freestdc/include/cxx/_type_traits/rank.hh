#ifndef _FREESTDC_CXX_TYPE_TRAITS_RANK_
#define _FREESTDC_CXX_TYPE_TRAITS_RANK_

#include <cstddef>
#include "integral_constant.hh"

namespace std {
	template <class T>
	struct rank : public std::integral_constant<size_t, 0> {};

	template <class T>
	struct rank<T[]> : public std::integral_constant<size_t, rank<T>::value + 1> {};

	template <class T, std::size_t N>
	struct rank<T[N]> : public std::integral_constant<size_t, rank<T>::value + 1> {};
}

#endif
