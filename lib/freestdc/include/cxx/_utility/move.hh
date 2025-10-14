#ifndef _FREESTDC_CXX_UTILITY_MOVE_
#define _FREESTDC_CXX_UTILITY_MOVE_

#include "_type_traits/add_reference.hh"
#include "_type_traits/remove_reference.hh"

namespace std {
	template <typename T>
	constexpr typename std::remove_reference<T>::type &&move(T &&t) noexcept {
		return static_cast<typename std::remove_reference<T>::type &&>(t);
	}

	template <typename T>
	typename std::add_rvalue_reference<T>::type declval() noexcept {
		static_assert(false, "declval is not allowed in an evaluated context");
	}
}

#endif
