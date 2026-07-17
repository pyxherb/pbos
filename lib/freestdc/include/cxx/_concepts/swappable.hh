#ifndef _FREESTDC_CXX_CONCEPTS_SWAPPABLE_
#define _FREESTDC_CXX_CONCEPTS_SWAPPABLE_

#include "common_ref.hh"
#include "../_utility/swap.hh"

namespace std {
	//
	// TODO: Use the range version to replace it.
	//
	template <class T>
	concept swappable =
		requires(T &a, T &b) {
			ranges::swap(a, b);
		};

	//
	// TODO: Use the range version to replace it.
	//
	template <class T, class U>
	concept swappable_with =
		std::common_reference_with<T, U> &&
		requires(T &&t, U &&u) {
			ranges::swap(std::forward<T>(t), std::forward<T>(t));
			ranges::swap(std::forward<U>(u), std::forward<U>(u));
			ranges::swap(std::forward<T>(t), std::forward<U>(u));
			ranges::swap(std::forward<U>(u), std::forward<T>(t));
		};
}

#endif
