#ifndef _FREESTDC_CXX_UTILITY_PAIR_
#define _FREESTDC_CXX_UTILITY_PAIR_

#include <functional>
#include "forward.hh"

namespace std {
	template <typename T1, typename T2>
	struct pair {
		T1 first;
		T2 second;

		constexpr explicit pair();
		constexpr explicit pair(const T1 &x, const T2 &y) : first(x), second(y) {}
		template <typename U1, typename U2>
		constexpr explicit pair(U1 &&x, U2 &&y) : first(std::forward(x)), second(std::forward(y)) {}
		template <typename U1, typename U2>
		constexpr explicit pair(const pair<U1, U2> &p) : first(p.first), second(p.second) {}
		template <typename U1, typename U2>
		constexpr explicit pair(pair<U1, U2> &&p) : first(std::forward(p.first)), second(std::forward(p.second)) {}

		// TODO: Implement the piecewise_construct cosntructor.

		pair(const pair &p) = default;
		pair(pair &&p) = default;
	};

	template <class T1, class T2>
	std::pair<T1, T2> make_pair(T1 x, T2 y) {
		return std::pair<T1, T2>(std::forward<T1>(x), std::forward<T2>(y));
	}

#if __cplusplus >= 202002L
	template <class T1, class T2>
	constexpr std::pair<std::unwrap_ref_decay_t<T1>, std::unwrap_ref_decay_t<T2>> make_pair(T1 &&x, T2 &&y) {
		// TODO: Implement it.
		static_assert(std::is_same_v<T1, T2>, "Unimplemented yet!");
	}
#endif
}

#endif
