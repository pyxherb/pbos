#ifndef _FREESTDC_CXX_ITERATOR_ITER_TYPES_
#define _FREESTDC_CXX_ITERATOR_ITER_TYPES_

#include "traits.hh"

namespace std {
	template <class T>
	using iter_difference_t = typename incrementable_traits<T>::difference_type;

	template <class T>
	using iter_value_t = typename indirectly_readable_traits<T>::value_type;

	template <class T>
	using iter_reference_t = decltype(*std::declval<T &>());

	namespace detail {
		using std::declval;

		template <class T>
		auto try_iter_move(int) -> decltype(iter_move(declval<T &>()));

		template <class T>
		auto try_iter_move(long) -> decltype(std::move(*declval<T &>()));

		template <class T>
		using rvalue_reference_type = decltype(try_iter_move<T>(0));
	}

	template <class T>
	using iter_rvalue_reference_t = detail::rvalue_reference_type<T>;

	template <class T>
	using iter_common_reference_t =
		std::common_reference_t<iter_reference_t<T>, iter_value_t<T> &>;

#if __cplusplus >= 202302L
	template <class T>
	using iter_const_reference_t =
		std::common_reference_t<const iter_value_t<T> &&, iter_reference_t<T>>;
#endif
}

#endif
