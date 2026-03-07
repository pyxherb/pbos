#ifndef _FREESTDC_CXX_UTILITY_FORWARD_
#define _FREESTDC_CXX_UTILITY_FORWARD_

#include "_type_traits/is_lvalue_reference.hh"
#include "_type_traits/remove_reference.hh"

namespace std {
	template <typename T>
	T &&forward(typename std::remove_reference<T>::type &t) noexcept {
		return static_cast<T &&>(t);
	}

	template <typename T>
	T &&forward(typename std::remove_reference<T>::type &&t) noexcept {
		static_assert(!std::is_lvalue_reference<T>::value,
			"Cannot forward an rvalue as an lvalue.");
		return static_cast<T &&>(t);
	}
}

#endif
