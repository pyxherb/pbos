#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_DESTRUCTIBLE_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_DESTRUCTIBLE_

#include <utility>
#include "add_cv.hh"
#include "integral_constant.hh"
#include "remove_extent.hh"
#include "void_t.hh"

namespace std {
	template <typename T>
	struct _has_destructor {
		template <typename U>
		static auto _impl(int) -> decltype(std::declval<U>().~U(), std::true_type{});

		template <typename U>
		static std::false_type _impl(...);

		using type = decltype(_impl<T>(0));
	};

	template <typename T>
	struct is_destructible : _has_destructor<T>::type {};

	template <>
	struct is_destructible<void> : false_type {};

	template <typename T>
	struct is_destructible<T[]> : false_type {};

	template <typename T, typename... Args>
	struct is_destructible<T(Args...)> : false_type {};

	template <typename, typename T, typename U = void>
	struct _is_trivially_destructible_impl : true_type {};

	template <typename T>
	struct _is_trivially_destructible_impl<
		void_t<_has_destructor<T>>,
		T> : false_type {};

#if defined(__clang__)
	#define _freestdc_has_trivial_destructor __is_trivially_destructible
#else
	#define _freestdc_has_trivial_destructor __has_trivial_destructor
#endif

	template <typename T>
	struct is_trivially_destructible : bool_constant<is_destructible<T>::value && _freestdc_has_trivial_destructor(T)> {};

	template <>
	struct is_trivially_destructible<void> : false_type {};

	template <typename T>
	struct is_trivially_destructible<T[]> : false_type {};

	template <typename T, typename... Args>
	struct is_trivially_destructible<T(Args...)> : false_type {};

	template <typename T>
	struct _has_nothrow_destructor {
		template <typename U>
		static auto _impl(int) -> decltype(std::declval<U>().~U(), std::true_type{});

		template <typename U>
		static std::false_type _impl(...);

		using type = bool_constant<is_trivially_destructible<T>::value || (decltype(_impl<T>(0))::value && noexcept(std::declval<T>().~T()))>;
	};

	template <typename T>
	struct is_nothrow_destructible : _has_nothrow_destructor<T>::type {};

	template <>
	struct is_nothrow_destructible<void> : false_type {};

	template <typename T>
	struct is_nothrow_destructible<T[]> : false_type {};

	template <typename T, typename... Args>
	struct is_nothrow_destructible<T(Args...)> : false_type {};

	template <typename T>
	constexpr bool is_destructible_v = is_destructible<T>::value;
	template <typename T>
	constexpr bool is_trivially_destructible_v = is_trivially_destructible<T>::value;
	template <typename T>
	constexpr bool is_nothrow_destructible_v = is_nothrow_destructible<T>::value;
}

#endif
