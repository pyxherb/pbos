#ifndef _FREESTDC_CXX_TYPE_TRAITS_ALIGNMENT_OF_
#define _FREESTDC_CXX_TYPE_TRAITS_ALIGNMENT_OF_

#include <cstddef>
#include "integral_constant.hh"

namespace std {
	template <class T>
	struct alignment_of : std::integral_constant<size_t, alignof(T)> {};
}

#endif
