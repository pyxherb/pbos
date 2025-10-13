#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_FINAL_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_FINAL_

#include "integral_constant.hh"

namespace std {
	template <typename T>
	struct is_final : std::bool_constant<__is_final(T)> {
	};

	#define __cpp_lib_is_final 201402L
}

#endif
