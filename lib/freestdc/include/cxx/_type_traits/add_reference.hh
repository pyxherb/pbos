#ifndef _FREESTDC_CXX_TYPE_TRAITS_ADD_REFERENCE_HH_
#define _FREESTDC_CXX_TYPE_TRAITS_ADD_REFERENCE_HH_

namespace std {
	template <typename T>
	struct _type_identity {
		using type = T;
	};

	template <typename T>
	auto _try_add_lvalue_reference(int) -> _type_identity<T &>;
	template <typename T>
	auto _try_add_lvalue_reference(...) -> _type_identity<T>;

	template <typename T>
	auto _try_add_rvalue_reference(int) -> _type_identity<T &&>;
	template <typename T>
	auto _try_add_rvalue_reference(...) -> _type_identity<T>;

	template <typename T>
	struct add_lvalue_reference
		: decltype(_try_add_lvalue_reference<T>(0)) {};

	template <typename T>
	struct add_rvalue_reference
		: decltype(_try_add_rvalue_reference<T>(0)) {};
}

#endif
