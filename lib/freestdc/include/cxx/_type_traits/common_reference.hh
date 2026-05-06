#ifndef _FREESTDC_CXX_TYPE_TRAITS_COMMON_REFERENCE_
#define _FREESTDC_CXX_TYPE_TRAITS_COMMON_REFERENCE_

#include "add_reference.hh"
#include "common_type.hh"
#include "is_convertible.hh"
#include "is_reference.hh"
#include "xref.hh"

namespace std {
	template <class T, class U, class TQual, class UQual, class = void>
	struct basic_common_reference {};

	template <class T, class U>
	using _basic_common_ref_t = typename basic_common_reference<
		std::remove_cvref<T>,
		std::remove_cvref<U>,
		_xref_t<T>,
		_xref_t<U>>::type;

	template <class...>
	struct common_reference;

	template <class T, class U>
	using _common_reference_t = typename common_reference<T, U>::type;

	template <class T, class U, class = void>
	struct _common_reference_ref {};

	template <class X, class Y>
	struct _common_reference_ref<X &, Y &, std::void_t<std::add_lvalue_reference_t<std::_common_reference_t<X, Y>>, std::enable_if_t<is_convertible_v<X &, std::add_lvalue_reference_t<_common_reference_t<X, Y>>> && std::is_convertible_v<Y &, add_lvalue_reference_t<_common_reference_t<X, Y>>>>>> {
		using type = add_lvalue_reference_t<_common_reference_t<X, Y>>;
	};

	template <class X, class Y>
	struct _common_reference_ref<X &&, Y &, std::void_t<std::enable_if_t<std::is_convertible_v<X &&, const Y &>>>> {
		using type = const Y &;
	};

	template <class X, class Y>
	struct _common_reference_ref<X &, Y &&, std::void_t<std::enable_if_t<std::is_convertible_v<Y &&, const X &>>>> {
		using type = const X &;
	};

	template <class X, class Y>
	struct _common_reference_ref<X &&, Y &&, std::void_t<std::enable_if_t<std::is_convertible_v<X &&, Y &&>>>> {
		using type = Y &&;
	};

	template <class X, class Y>
	struct _common_reference_ref<Y &&, X &&, std::void_t<std::enable_if_t<std::is_convertible_v<Y &&, X &&>>>> {
		using type = X &&;
	};

	template <class T, class U, class = void>
	struct _common_reference_impl {};

	template <class T, class U>
	struct _common_reference_impl<T, U, std::enable_if_t<std::is_reference_v<T> && std::is_reference_v<U>>> {
		using type = typename _common_reference_ref<T, U>::type;
	};

	template <class T, class U>
	struct _common_reference_impl<T, U, void_t<_basic_common_ref_t<T, U>>> {
		using type = _basic_common_ref_t<T, U>;
	};

	template <class T, class U>
	struct _common_reference_impl<T, U, std::void_t<typename std::common_type<T, U>::type>> {
		using type = typename common_type<T, U>::type;
	};

	template <class T>
	struct common_reference<T> {
		using type = T;
	};

	template <class T, class U>
	struct common_reference<T, U> : _common_reference_impl<T, U> {};

	template <class... T>
	using common_reference_t = typename common_reference<T...>::type;

	template <class T1, class T2, class... Ts>
	struct common_reference<T1, T2, Ts...>
		: common_reference<T1, common_reference_t<T2, Ts...>> {};
}

#endif
