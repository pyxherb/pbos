#ifndef _FREESTDC_CXX_TYPE_TRAITS_VOID_T_
#define _FREESTDC_CXX_TYPE_TRAITS_VOID_T_

namespace std {
	template <typename... Ts>
	struct _make_void {
		typedef void type;
	};

	template <typename... Ts>
	using void_t = typename _make_void<Ts...>::type;

	#define __cpp_lib_void_t 201411L
}

#endif
