#ifndef _FREESTDC_CXX_TYPE_TRAITS_REMOVE_EXTENT_
#define _FREESTDC_CXX_TYPE_TRAITS_REMOVE_EXTENT_

#include <cstddef>

namespace std {
	template <class T>
	struct remove_extent {
		using type = T;
	};

	template <class T>
	struct remove_extent<T[]> {
		using type = T;
	};

	template <class T, size_t N>
	struct remove_extent<T[N]> {
		using type = T;
	};

	template <class T>
	struct remove_all_extents {
		typedef T type;
	};

	template <class T>
	struct remove_all_extents<T[]> {
		typedef typename remove_all_extents<T>::type type;
	};

	template <class T, std::size_t N>
	struct remove_all_extents<T[N]> {
		typedef typename remove_all_extents<T>::type type;
	};
}

#endif
