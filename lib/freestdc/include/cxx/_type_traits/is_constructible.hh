#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_CONSTRUCTIBLE_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_CONSTRUCTIBLE_

#include <utility>
#include "add_cv.hh"
#include "integral_constant.hh"
#include "void_t.hh"

namespace std {
	template <typename, typename T, typename... Args>
	struct _is_constructible_impl : false_type {};

	template <typename T, typename... Args>
	struct _is_constructible_impl<
		void_t<decltype(T(declval<Args>()...))>,
		T,
		Args...> : true_type {};

	template <typename T, typename... Args>
	struct is_constructible : _is_constructible_impl<void_t<>, T, Args...> {};

	template <typename T, typename... Args>
	struct is_trivially_constructible : integral_constant<bool, __is_trivially_constructible(T, Args...)> {};

	template <typename, typename T, typename... Args>
	struct _is_nothrow_constructible_impl : false_type {};

	template <typename T, typename... Args>
	struct _is_nothrow_constructible_impl<
		void_t<decltype(T(declval<Args>()...))>,
		T,
		Args...> : bool_constant<noexcept(T(declval<Args>()...))> {};

	template <typename T, typename... Args>
	struct is_nothrow_constructible : _is_nothrow_constructible_impl<void_t<>, T, Args...> {};

	template <typename T>
	struct is_default_constructible : is_constructible<T> {};

	template <typename T>
	struct is_trivially_default_constructible : is_trivially_constructible<T> {};

	template <typename T>
	struct is_nothrow_default_constructible : is_nothrow_constructible<T> {};

	template <typename T>
	struct is_copy_constructible : is_constructible<T, typename add_lvalue_reference<typename add_const<T>::type>::type> {};

	template <typename T>
	struct is_trivially_copy_constructible : is_trivially_constructible<T, typename add_lvalue_reference<typename add_const<T>::type>::type> {};

	template <typename T>
	struct is_nothrow_copy_constructible : is_nothrow_constructible<T, typename add_lvalue_reference<typename add_const<T>::type>::type> {};

	template <typename T>
	struct is_move_constructible : is_constructible<T, typename add_rvalue_reference<T>::type> {};

	template <typename T>
	struct is_trivially_move_constructible : is_trivially_constructible<T, typename add_rvalue_reference<T>::type> {};

	template <typename T>
	struct is_nothrow_move_constructible : is_nothrow_constructible<T, typename add_rvalue_reference<T>::type> {};

	template <typename T, typename... Args>
	inline constexpr bool is_constructible_v =
		is_constructible<T, Args...>::value;
	template <typename T, typename... Args>
	inline constexpr bool is_trivially_constructible_v =
		is_trivially_constructible<T, Args...>::value;
	template <typename T, typename... Args>
	inline constexpr bool is_nothrow_constructible_v =
		is_nothrow_constructible<T, Args...>::value;
	template <typename T>
	inline constexpr bool is_default_constructible_v =
		is_default_constructible<T>::value;
	template <typename T>
	inline constexpr bool is_trivially_default_constructible_v =
		is_trivially_default_constructible<T>::value;
	template <typename T>
	inline constexpr bool is_nothrow_default_constructible_v =
		is_nothrow_default_constructible<T>::value;
	template <typename T>
	inline constexpr bool is_copy_constructible_v =
		is_copy_constructible<T>::value;
	template <typename T>
	inline constexpr bool is_trivially_copy_constructible_v =
		is_trivially_copy_constructible<T>::value;
	template <typename T>
	inline constexpr bool is_nothrow_copy_constructible_v =
		is_nothrow_copy_constructible<T>::value;
	template <typename T>
	inline constexpr bool is_move_constructible_v =
		is_move_constructible<T>::value;
	template <typename T>
	inline constexpr bool is_trivially_move_constructible_v =
		is_trivially_move_constructible<T>::value;
	template <typename T>
	inline constexpr bool is_nothrow_move_constructible_v =
		is_nothrow_move_constructible<T>::value;
}

#endif
