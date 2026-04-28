#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_CONSTANT_EVALUATED_HH_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_CONSTANT_EVALUATED_HH_

namespace std {
	constexpr bool is_constant_evaluated() noexcept {
		// Undocumented in Clang 23's official document.
		return __builtin_is_constant_evaluated();
	}
}

#endif
