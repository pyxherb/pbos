#ifndef _FREESTDC_CONCEPTS_ASSIGNABLE_FROM_HH_
#define _FREESTDC_CONCEPTS_ASSIGNABLE_FROM_HH_

#include "common_ref.hh"

namespace std {
	template <class LHS, class RHS>
	concept assignable_from =
		std::is_lvalue_reference_v<LHS> &&
		std::common_reference_with<
			const std::remove_reference_t<LHS> &,
			const std::remove_reference_t<RHS> &> &&
		requires(LHS lhs, RHS &&rhs) {
			{ lhs = std::forward<RHS>(rhs) } -> std::same_as<LHS>;
		};
}

#endif
