#ifndef _FREESTDC_CXX_TYPE_TRAITS_UNDERLYING_TYPE_
#define _FREESTDC_CXX_TYPE_TRAITS_UNDERLYING_TYPE_

namespace std {
	template <typename T>
	struct underlying_type {
		typedef __underlying_type(T) type;
	};

	template <typename T>
	using underlying_type_t = typename underlying_type<T>::type;
}

#endif
