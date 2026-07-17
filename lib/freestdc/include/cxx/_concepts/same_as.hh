#ifndef _FREESTDC_CXX_CONCEPTS_SAME_AS_
#define _FREESTDC_CXX_CONCEPTS_SAME_AS_

#include "../_type_traits/is_same.hh"

namespace std {
	template <typename T, typename U>
	concept same_as = std::is_same_v<T, U> && std::is_same_v<U, T>;
}

#endif
