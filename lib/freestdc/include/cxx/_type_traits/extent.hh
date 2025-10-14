#ifndef _FREESTDC_CXX_TYPE_TRAITS_EXTENT_
#define _FREESTDC_CXX_TYPE_TRAITS_EXTENT_

#include <cstddef>
#include "integral_constant.hh"

namespace std {
	template <typename T, unsigned N = 0>
	struct extent : std::integral_constant<size_t, 0> {};

	template <typename T>
	struct extent<T[], 0> : std::integral_constant<size_t, 0> {};

	template <typename T, unsigned N>
	struct extent<T[], N> : std::extent<T, N - 1> {};

	template <typename T, size_t I>
	struct extent<T[I], 0> : std::integral_constant<size_t, I> {};

	template <typename T, size_t I, unsigned N>
	struct extent<T[I], N> : std::extent<T, N - 1> {};
}

#endif
