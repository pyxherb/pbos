#ifndef _FREESTDC_CXX_ITERATOR_INDIRECT_CALL_
#define _FREESTDC_CXX_ITERATOR_INDIRECT_CALL_

#include "concepts.hh"

namespace std {
	// indirectly_unary_invocable
	template <class F, class I>
	concept indirectly_unary_invocable =
		indirectly_readable<I> &&
		copy_constructible<F> &&
		invocable<F &, iter_value_t<I> &> &&
		invocable<F &, iter_reference_t<I>> &&
		invocable<F &, iter_common_reference_t<I>> &&
		common_reference_with<
			invoke_result_t<F &, iter_value_t<I> &>,
			invoke_result_t<F &, iter_reference_t<I>>>;

	// indirectly_regular_unary_invocable
	template <class F, class I>
	concept indirectly_regular_unary_invocable =
		indirectly_readable<I> &&
		copy_constructible<F> &&
		regular_invocable<F &, iter_value_t<I> &> &&
		regular_invocable<F &, iter_reference_t<I>> &&
		regular_invocable<F &, iter_common_reference_t<I>> &&
		common_reference_with<
			invoke_result_t<F &, iter_value_t<I> &>,
			invoke_result_t<F &, iter_reference_t<I>>>;

	// indirect_unary_predicate
	template <class F, class I>
	concept indirect_unary_predicate =
		indirectly_regular_unary_invocable<F, I> &&
		predicate<F &, iter_value_t<I> &> &&
		predicate<F &, iter_reference_t<I>> &&
		predicate<F &, iter_common_reference_t<I>>;

	// indirect_binary_predicate
	template <class F, class I1, class I2>
	concept indirect_binary_predicate =
		indirectly_readable<I1> &&
		indirectly_readable<I2> &&
		copy_constructible<F> &&
		predicate<F &, iter_value_t<I1> &, iter_value_t<I2> &> &&
		predicate<F &, iter_value_t<I1> &, iter_reference_t<I2>> &&
		predicate<F &, iter_reference_t<I1>, iter_value_t<I2> &> &&
		predicate<F &, iter_reference_t<I1>, iter_reference_t<I2>> &&
		predicate<F &, iter_common_reference_t<I1>, iter_common_reference_t<I2>>;

	// indirect_equivalence_relation
	template <class F, class I1, class I2>
	concept indirect_equivalence_relation =
		indirect_binary_predicate<F, I1, I2> &&
		equivalence_relation<F &, iter_value_t<I1> &, iter_value_t<I2> &> &&
		equivalence_relation<F &, iter_value_t<I1> &, iter_reference_t<I2>> &&
		equivalence_relation<F &, iter_reference_t<I1>, iter_value_t<I2> &> &&
		equivalence_relation<F &, iter_reference_t<I1>, iter_reference_t<I2>> &&
		equivalence_relation<F &, iter_common_reference_t<I1>, iter_common_reference_t<I2>>;

	// indirect_strict_weak_order
	template <class F, class I1, class I2 = I1>
	concept indirect_strict_weak_order =
		indirect_binary_predicate<F, I1, I2> &&
		strict_weak_order<F &, iter_value_t<I1> &, iter_value_t<I2> &> &&
		strict_weak_order<F &, iter_value_t<I1> &, iter_reference_t<I2>> &&
		strict_weak_order<F &, iter_reference_t<I1>, iter_value_t<I2> &> &&
		strict_weak_order<F &, iter_reference_t<I1>, iter_reference_t<I2>> &&
		strict_weak_order<F &, iter_common_reference_t<I1>, iter_common_reference_t<I2>>;
}

#endif
