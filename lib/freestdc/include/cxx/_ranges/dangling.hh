#ifndef _FREESTDC_CXX_RANGES_DANGLING_
#define _FREESTDC_CXX_RANGES_DANGLING_

#include "concepts.hh"
#include "view.hh"

namespace std {
	namespace ranges {
		struct dangling {
			constexpr dangling() noexcept = default;
			template <class... Args>
			constexpr dangling(Args &&...) noexcept {}
		};

		template <std::ranges::range R>
		using borrowed_iterator_t = std::conditional_t<std::ranges::borrowed_range<R>,
			std::ranges::iterator_t<R>,
			std::ranges::dangling>;

		template <std::ranges::range R>
		using borrowed_subrange_t = std::conditional_t<std::ranges::borrowed_range<R>,
			std::ranges::subrange<std::ranges::iterator_t<R>>,
			std::ranges::dangling>;
	}
}

#endif
