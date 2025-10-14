#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_ASSIGNABLE_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_ASSIGNABLE_

#include <utility>
#include "add_cv.hh"
#include "integral_constant.hh"
#include "void_t.hh"

namespace std {
	template <typename, typename T, typename U>
	struct _is_assignable_impl : false_type {};

	template <typename T, typename U>
	struct _is_assignable_impl<
		void_t<decltype(declval<T>() = declval<U>())>,
		T,
		U> : true_type {};

	template <typename T, typename... Args>
	struct is_assignable : _is_assignable_impl<void_t<>, T, Args...> {};

	template <typename T, typename... Args>
	struct is_trivially_assignable : integral_constant<bool, __is_trivially_assignable(T, Args...)> {};

	template <typename, typename T, typename U>
	struct _is_nothrow_assignable_impl : false_type {};

	template <typename T, typename U>
	struct _is_nothrow_assignable_impl<
		void_t<decltype(declval<T>() = declval<U>())>,
		T,
		U> : bool_constant<noexcept(declval<T>() = declval<U>())> {};

	template <typename T, typename... Args>
	struct is_nothrow_assignable : _is_nothrow_assignable_impl<void_t<>, T, Args...> {};

	template <typename T>
	struct is_copy_assignable
		: std::is_assignable<typename std::add_lvalue_reference<T>::type,
			  typename std::add_lvalue_reference<const T>::type> {};

	template <typename T>
	struct is_trivially_copy_assignable
		: std::is_trivially_assignable<typename std::add_lvalue_reference<T>::type,
			  typename std::add_lvalue_reference<const T>::type> {};

	template <typename T>
	struct is_nothrow_copy_assignable
		: std::is_nothrow_assignable<typename std::add_lvalue_reference<T>::type,
			  typename std::add_lvalue_reference<const T>::type> {};

	template <typename T>
	struct is_move_assignable
		: std::is_assignable<typename std::add_lvalue_reference<T>::type,
			  typename std::add_rvalue_reference<T>::type> {};

	template <typename T>
	struct is_trivially_move_assignable
		: std::is_trivially_assignable<typename std::add_lvalue_reference<T>::type,
			  typename std::add_rvalue_reference<T>::type> {};

	template <typename T>
	struct is_nothrow_move_assignable
		: std::is_nothrow_assignable<typename std::add_lvalue_reference<T>::type,
			  typename std::add_rvalue_reference<T>::type> {};

	template <typename T, typename U>
	constexpr bool is_assignable_v = is_assignable<T, U>::value;
	template <typename T, typename U>
	constexpr bool is_trivially_assignable_v = is_trivially_assignable<T, U>::value;
	template <typename T, typename U>
	constexpr bool is_nothrow_assignable_v = is_nothrow_assignable<T, U>::value;
	template <typename T>
	inline constexpr bool is_copy_assignable_v =
		is_copy_assignable<T>::value;
	template <typename T>
	inline constexpr bool is_trivially_copy_assignable_v =
		is_trivially_copy_assignable<T>::value;
	template <typename T>
	inline constexpr bool is_nothrow_copy_assignable_v =
		is_nothrow_copy_assignable<T>::value;
	template <typename T>
	inline constexpr bool is_move_assignable_v =
		is_move_assignable<T>::value;
	template <typename T>
	inline constexpr bool is_trivially_move_assignable_v =
		is_trivially_move_assignable<T>::value;
	template <typename T>
	inline constexpr bool is_nothrow_move_assignable_v =
		is_nothrow_move_assignable<T>::value;
}

#endif
