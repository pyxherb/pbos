#ifndef _FREESTDC_CXX_TYPE_TRAITS_CONJUNCTION_
#define _FREESTDC_CXX_TYPE_TRAITS_CONJUNCTION_

#include "conditional.hh"
#include "integral_constant.hh"

namespace std {
	template <typename...>
	struct conjunction : std::true_type {};

	template <typename B1>
	struct conjunction<B1> : B1 {};

	template <typename B1, typename... Bn>
	struct conjunction<B1, Bn...>
		: std::conditional<bool(B1::value), conjunction<Bn...>, B1>::type {};

	template <class... B>
	constexpr bool conjunction_v = conjunction<B...>::value;
}

#endif
