#ifndef _FREESTDC_CXX_CONCEPTS_DERIVED_FROM_
#define _FREESTDC_CXX_CONCEPTS_DERIVED_FROM_

#include <type_traits>

namespace std {
	template <typename Derived, typename Base>
	concept derived_from =
		std::is_base_of_v<Base, Derived> &&
		std::is_convertible_v<const volatile Derived *, const volatile Base *>;
}

#endif
