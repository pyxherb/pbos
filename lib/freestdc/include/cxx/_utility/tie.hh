#ifndef _FREESTDC_CXX_UTILITY_TIE_
#define _FREESTDC_CXX_UTILITY_TIE_

#include "tuple.hh"

namespace std {
	template <typename... Args>
	constexpr  // since C++14
		std::tuple<Args &...>
		tie(Args &...args) noexcept {
		return { args... };
	}

#define __cpp_lib_integer_sequence 201304L
}

#endif
