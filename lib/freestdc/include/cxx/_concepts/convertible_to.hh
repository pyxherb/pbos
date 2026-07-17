#ifndef _FREESTDC_CXX_CONCEPTS_CONVERTIBLE_TO_
#define _FREESTDC_CXX_CONCEPTS_CONVERTIBLE_TO_

#include "../_type_traits/is_convertible.hh"

namespace std {
	template <class From, class To>
	concept convertible_to =
		std::is_convertible_v<From, To> &&
		requires {
			static_cast<To>(std::declval<From>());
		};
}

#endif
