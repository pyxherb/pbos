#ifndef _FREESTDC_CXX_TYPE_TRAITS_NEGATION_
#define _FREESTDC_CXX_TYPE_TRAITS_NEGATION_

#include "conditional.hh"
#include "integral_constant.hh"

namespace std {
	template <typename B>
	struct negation : std::bool_constant<!bool(B::value)> {};

	template <typename B>
	constexpr bool negation_v = negation<B>::value;

#define __cpp_lib_logical_traits 201510L
}

#endif
