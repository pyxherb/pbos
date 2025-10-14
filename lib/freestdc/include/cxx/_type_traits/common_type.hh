#ifndef _FREESTDC_CXX_TYPE_TRAITS_COMMON_TYPE_
#define _FREESTDC_CXX_TYPE_TRAITS_COMMON_TYPE_

#include <utility>
#include "conditional.hh"
#include "decay.hh"
#include "is_same.hh"

namespace std {
	template <typename...>
	struct common_type {};

	// 1 type
	template <typename T>
	struct common_type<T> : common_type<T, T> {};

	template <typename T1, typename T2>
	using _conditional_result_t = decltype(false ? declval<T1>() : declval<T2>());

	template <typename, typename, typename = void>
	struct _decay_conditional_result {};
	template <typename T1, typename T2>
	struct _decay_conditional_result<T1, T2, void_t<_conditional_result_t<T1, T2>>>
		: decay<_conditional_result_t<T1, T2>> {};

	template <typename T1, typename T2, typename = void>
	struct _common_type_2_impl : _decay_conditional_result<const T1 &, const T2 &> {};

	template <typename T1, typename T2>
	struct _common_type_2_impl<T1, T2, void_t<_conditional_result_t<T1, T2>>>
		: _decay_conditional_result<T1, T2> {};

	// 2 types.
	template <typename T1, typename T2>
	struct common_type<T1, T2>
		: conditional<is_same<T1, typename decay<T1>::type>::value &&
						  is_same<T2, typename decay<T2>::type>::value,
			  _common_type_2_impl<T1, T2>,
			  common_type<typename decay<T1>::type,
				  typename decay<T2>::type>>::type {};

	// More than 3 types.
	template <typename AlwaysVoid, typename T1, typename T2, typename... R>
	struct _common_type_multi_impl {};
	template <typename T1, typename T2, typename... R>
	struct _common_type_multi_impl<void_t<typename common_type<T1, T2>::type>, T1, T2, R...>
		: common_type<typename common_type<T1, T2>::type, R...> {};

	template <typename T1, typename T2, typename... R>
	struct common_type<T1, T2, R...>
		: _common_type_multi_impl<void, T1, T2, R...> {};
}

#endif
