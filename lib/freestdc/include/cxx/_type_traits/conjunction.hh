#ifndef _FREESTDC_CXX_TYPE_TRAITS_CONJUNCTION_
#define _FREESTDC_CXX_TYPE_TRAITS_CONJUNCTION_

#include "integral_constant.hh"
#include "conditional.hh"

namespace std {
	template <typename...>
	struct conjunction : std::true_type {};

	template <typename B1>
	struct conjunction<B1> : B1 {};

	template <typename B1, typename... Bn>
	struct conjunction<B1, Bn...>
		: std::conditional<bool(B1::value), conjunction<Bn...>, B1>::type {};
}

#endif
