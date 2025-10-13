#ifndef _FREESTDC_CXX_TYPE_TRAITS_CONDITIONAL_
#define _FREESTDC_CXX_TYPE_TRAITS_CONDITIONAL_

namespace std {
	template <bool B, class T, class F>
	struct conditional {
		using type = T;
	};

	template <class T, class F>
	struct conditional<false, T, F> {
		using type = F;
	};
}

#endif
