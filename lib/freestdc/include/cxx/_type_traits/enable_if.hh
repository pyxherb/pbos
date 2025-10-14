#ifndef _FREESTDC_CXX_TYPE_TRAITS_ENABLE_IF_
#define _FREESTDC_CXX_TYPE_TRAITS_ENABLE_IF_

namespace std {
	template <bool B, class T = void>
	struct enable_if {};

	template <class T>
	struct enable_if<true, T> {
		typedef T type;
	};
}

#endif
