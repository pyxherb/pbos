#ifndef _FREESTDC_CXX_ITERATOR_UTILS_
#define _FREESTDC_CXX_ITERATOR_UTILS_

#include "indirect_call.hh"

namespace std {
	template <class F, class... Is>
		requires(std::indirectly_readable<Is> && ...) &&
					std::invocable<F, std::iter_reference_t<Is>...>
	using indirect_result_t = std::invoke_result_t<F, std::iter_reference_t<Is>...>;

	// TODO: Support C++26.
	template <std::indirectly_readable I,
		std::indirectly_regular_unary_invocable<I> Proj>
	struct projected {
		using value_type = std::remove_cvref_t<std::indirect_result_t<Proj &, I>>;
		std::indirect_result_t<Proj &, I> operator*() const;  // not defined
	};

	template <std::weakly_incrementable I, class Proj>
	struct incrementable_traits<std::projected<I, Proj>> {
		using difference_type = std::iter_difference_t<I>;
	};

	template <std::indirectly_readable I,
		std::indirectly_regular_unary_invocable<I> Proj>
	using projected_value_t =
		std::remove_cvref_t<std::invoke_result_t<Proj &, std::iter_value_t<I> &>>;
}

#endif
