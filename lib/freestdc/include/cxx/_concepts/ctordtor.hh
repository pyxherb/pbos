#ifndef _FREESTDC_CXX_CONCEPTS_CTORDTOR_
#define _FREESTDC_CXX_CONCEPTS_CTORDTOR_

#include "convertible_to.hh"
#include <type_traits>

namespace std {
	template <class T>
	concept destructible = std::is_nothrow_destructible_v<T>;

	template <class T, class... Args>
	concept constructible_from =
		std::destructible<T> && std::is_constructible_v<T, Args...>;

	template <class T>
	concept default_initializable =
		std::constructible_from<T> &&
		requires { T{}; ::new T; };

	template <class T>
	concept move_constructible = std::constructible_from<T, T> && std::convertible_to<T, T>;

	template <class T>
	concept copy_constructible =
		std::move_constructible<T> &&
		std::constructible_from<T, T &> && std::convertible_to<T &, T> &&
		std::constructible_from<T, const T &> && std::convertible_to<const T &, T> &&
		std::constructible_from<T, const T> && std::convertible_to<const T, T>;
}

#endif
