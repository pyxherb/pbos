#ifndef _FREESTDC_CXX_TYPE_TRAITS_ADD_REFERENCE_HH_
#define _FREESTDC_CXX_TYPE_TRAITS_ADD_REFERENCE_HH_

namespace std {
	template <typename T>
	struct _add_reference_type_identity {
		using type = T;
	};

	template <typename T>
	auto _try_add_lvalue_reference(int) -> _add_reference_type_identity<T &>;
	template <typename T>
	auto _try_add_lvalue_reference(...) -> _add_reference_type_identity<T>;

	template <typename T>
	auto _try_add_rvalue_reference(int) -> _add_reference_type_identity<T &&>;
	template <typename T>
	auto _try_add_rvalue_reference(...) -> _add_reference_type_identity<T>;

	template <typename T>
	struct add_lvalue_reference
		: decltype(_try_add_lvalue_reference<T>(0)) {};

	template <typename T>
	struct add_rvalue_reference
		: decltype(_try_add_rvalue_reference<T>(0)) {};

	template<typename T>
	using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

	template<typename T>
	using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;
}

#endif
