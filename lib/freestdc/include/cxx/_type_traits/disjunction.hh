#ifndef _FREESTDC_CXX_TYPE_TRAITS_DISJUNCTION_
#define _FREESTDC_CXX_TYPE_TRAITS_DISJUNCTION_

#include "conditional.hh"
#include "integral_constant.hh"

namespace std {
	template <typename...>
	struct disjunction : std::false_type {};

	template <typename B1>
	struct disjunction<B1> : B1 {};

	template <typename B1, typename... Bn>
	struct disjunction<B1, Bn...>
		: std::conditional<bool(B1::value), B1, disjunction<Bn...>>::type {};
}

#endif
