#ifndef _FREESTDC_CXX_ITERATOR_REQUIREMENTS_
#define _FREESTDC_CXX_ITERATOR_REQUIREMENTS_

#include "indirect_call.hh"
#include "iter_move.hh"
#include "utils.hh"
#include "../_functional/op.hh"

namespace std {
	template <class In, class Out>
	concept indirectly_movable =
		std::indirectly_readable<In> &&
		std::indirectly_writable<Out, std::iter_rvalue_reference_t<In>>;

	template <class In, class Out>
	concept indirectly_movable_storable =
		std::indirectly_movable<In, Out> &&
		std::indirectly_writable<Out, std::iter_value_t<In>> &&
		std::movable<std::iter_value_t<In>> &&
		std::constructible_from<std::iter_value_t<In>, std::iter_rvalue_reference_t<In>> &&
		std::assignable_from<std::iter_value_t<In> &, std::iter_rvalue_reference_t<In>>;

	template <class In, class Out>
	concept indirectly_copyable =
		std::indirectly_readable<In> &&
		std::indirectly_writable<Out, std::iter_reference_t<In>>;

	template <class In, class Out>
	concept indirectly_copyable_storable =
		std::indirectly_copyable<In, Out> &&
		std::indirectly_writable<Out, std::iter_value_t<In> &> &&
		std::indirectly_writable<Out, const std::iter_value_t<In> &> &&
		std::indirectly_writable<Out, std::iter_value_t<In> &&> &&
		std::indirectly_writable<Out, const std::iter_value_t<In> &&> &&
		std::copyable<std::iter_value_t<In>> &&
		std::constructible_from<std::iter_value_t<In>, std::iter_reference_t<In>> &&
		std::assignable_from<std::iter_value_t<In> &, std::iter_reference_t<In>>;

	template <class I1, class I2 = I1>
	concept indirectly_swappable =
		std::indirectly_readable<I1> &&
		std::indirectly_readable<I2> &&
		requires(const I1 i1, const I2 i2) {
			ranges::iter_swap(i1, i1);
			ranges::iter_swap(i1, i2);
			ranges::iter_swap(i2, i1);
			ranges::iter_swap(i2, i2);
		};

	template <class I1, class I2, class Comp, class Proj1 = std::identity, class Proj2 = std::identity>
	concept indirectly_comparable =
		std::indirect_binary_predicate<Comp, std::projected<I1, Proj1>, std::projected<I2, Proj2>>;

	template <class I>
	concept permutable =
		std::forward_iterator<I> &&
		std::indirectly_movable_storable<I, I> &&
		std::indirectly_swappable<I, I>;

	template <class I1, class I2, class Out, class Comp = ranges::less, class Proj1 = std::identity, class Proj2 = std::identity>
	concept mergeable =
		std::input_iterator<I1> &&
		std::input_iterator<I2> &&
		std::weakly_incrementable<Out> &&
		std::indirectly_copyable<I1, Out> &&
		std::indirectly_copyable<I2, Out> &&
		std::indirect_strict_weak_order<Comp,
			std::projected<I1, Proj1>,
			std::projected<I2, Proj2>>;

	template <class I, class Comp = ranges::less, class Proj = std::identity>
	concept sortable =
		std::permutable<I> &&
		std::indirect_strict_weak_order<Comp, std::projected<I, Proj>>;
}

#endif
