#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_ENUM_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_ENUM_

namespace std {
	template <typename T>
	struct is_enum {
		constexpr static bool value = __is_enum(T);
	};
}

#endif
