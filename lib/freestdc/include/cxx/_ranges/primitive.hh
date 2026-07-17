#ifndef _FREESTDC_CXX_RANGES_PRIMITIVE_
#define _FREESTDC_CXX_RANGES_PRIMITIVE_

#include "../_iterator/iter_types.hh"
#include "access.hh"
#include "concepts.hh"

namespace std {
	namespace ranges {
		template <class T>
		using iterator_t = decltype(ranges::begin(std::declval<T &>()));

#if __cplusplus >= 202302L
		template <ranges::range R>
		using const_iterator_t = decltype(ranges::cbegin(std::declval<R &>()));
#endif

#if __cplusplus >= 202302L
		template <ranges::range R>
		using const_sentinel_t = decltype(ranges::cend(std::declval<R &>()));
#endif

		template <ranges::sized_range R>
		using range_size_t = decltype(ranges::size(std::declval<R &>()));

#if __cplusplus >= 202302L
		template <ranges::range R>
		using range_const_reference_t =
			std::iter_const_reference_t<ranges::iterator_t<R>>;
#endif

		template <ranges::range R>
		using range_common_reference_t =
			std::iter_common_reference_t<ranges::iterator_t<R>>;
	}
}

#endif
