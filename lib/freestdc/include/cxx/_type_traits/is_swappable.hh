#ifndef _FREESTDC_CXX_TYPE_TRAITS_IS_SWAPPABLE_
#define _FREESTDC_CXX_TYPE_TRAITS_IS_SWAPPABLE_

#include <algorithm>
#include <utility>

namespace std {
	template <typename T, typename U>
	struct _is_swappable_with_impl {
		template <typename V>
		static auto _impl(int) -> decltype(std::swap(declval<T>(), declval<U>()), std::swap(declval<U>(), declval<T>()), std::true_type{});

		template <typename V>
		static std::false_type _impl(...);

		using type = decltype(_impl<void>(0));
	};

	template <typename T, typename U>
	struct is_swappable_with : _is_swappable_with_impl<T, U>::type {
	};

	template <typename T>
	struct _is_swappable_impl {
		template <typename V>
		static auto _impl(int) -> decltype(std::swap(declval<T>(), declval<T>()), std::true_type{});

		template <typename V>
		static std::false_type _impl(...);

		using type = decltype(_impl<void>(0));
	};

	template <typename T>
	struct is_swappable : _is_swappable_impl<T>::type {
	};

	template <typename T, typename U>
	struct _is_nothrow_swappable_with_impl {
		template <typename V>
		static auto _impl(int) -> decltype(std::swap(declval<T>(), declval<U>()), std::swap(declval<U>(), declval<T>()), std::true_type{});

		template <typename V>
		static std::false_type _impl(...);

		using type = bool_constant<decltype(_impl<void>(0))::value && noexcept(std::swap(declval<T>(), declval<U>())) && noexcept(std::swap(declval<U>(), declval<T>()))>;
	};

	template <typename T, typename U>
	struct is_nothrow_swappable_with : _is_nothrow_swappable_with_impl<T, U>::type {
	};

	template <typename T>
	struct _is_nothrow_swappable_impl {
		template <typename V>
		static auto _impl(int) -> decltype(std::swap(declval<T>(), declval<T>()), std::true_type{});

		template <typename V>
		static std::false_type _impl(...);

		using type = bool_constant<decltype(_impl<void>(0))::value && noexcept(std::swap(declval<T>(), declval<T>()))>;
	};

	template <typename T>
	struct is_nothrow_swappable : _is_nothrow_swappable_impl<T>::type {
	};

	template <typename T, typename U>
	inline constexpr bool is_swappable_with_v = is_swappable_with<T, U>::value;
	template <typename T>
	inline constexpr bool is_swappable_v = is_swappable<T>::value;
	template <typename T, typename U>
	inline constexpr bool is_nothrow_swappable_with_v =
		is_nothrow_swappable_with<T, U>::value;
	template <typename T>
	inline constexpr bool is_nothrow_swappable_v =
		is_nothrow_swappable<T>::value;
}

#endif
