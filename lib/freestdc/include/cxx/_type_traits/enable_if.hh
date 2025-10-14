#ifndef _FREESTDC_CXX_TYPE_TRAITS_ENABLE_IF_
#define _FREESTDC_CXX_TYPE_TRAITS_ENABLE_IF_

namespace std {
	template <bool B, typename T = void>
	struct enable_if {};

	template <typename T>
	struct enable_if<true, T> {
		typedef T type;
	};

	template <bool B, typename T = void>
	using enable_if_t = typename enable_if<B, T>::type;
}

#endif
