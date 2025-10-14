#ifndef _FREESTDC_CXX_TYPE_TRAITS_CONDITIONAL_
#define _FREESTDC_CXX_TYPE_TRAITS_CONDITIONAL_

namespace std {
	template <bool B, typename T, typename F>
	struct conditional {
		using type = T;
	};

	template <typename T, typename F>
	struct conditional<false, T, F> {
		using type = F;
	};
}

#endif
