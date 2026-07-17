#ifndef _FREESTDC_CXX_ITERATOR_CONCEPTS_
#define _FREESTDC_CXX_ITERATOR_CONCEPTS_

#include "../_concepts/arithm_traits.hh"
#include "../_concepts/common_ref.hh"
#include "../_concepts/derived_from.hh"
#include "../_concepts/misc.hh"
#include "iter_types.hh"

namespace std {

	template <class In>
	concept indirectly_readable =
		requires {
			typename iter_value_t<In>;
			typename iter_reference_t<In>;
			typename iter_rvalue_reference_t<In>;
			typename iter_common_reference_t<In>;
		} &&
		common_reference_with<iter_reference_t<In> &&, iter_value_t<In> &> &&
		common_reference_with<iter_reference_t<In> &&, iter_rvalue_reference_t<In> &&> &&
		common_reference_with<iter_rvalue_reference_t<In> &&, const iter_value_t<In> &>;

	template <class Out, class T>
	concept indirectly_writable =
		requires(Out &&o, T &&t) {
			*o = std::forward<T>(t);
			*std::forward<Out>(o) = std::forward<T>(t);
			const_cast<const iter_reference_t<Out> &&>(*o) = std::forward<T>(t);
			const_cast<const iter_reference_t<Out> &&>(*std::forward<Out>(o)) = std::forward<T>(t);
		};

	template <class T>
	concept _signed_integer_like = signed_integral<T>;

	template <class T>
	concept weakly_incrementable =
		is_object_v<T> && regular<T> &&
		requires(T t) {
			{ ++t } -> same_as<T &>;
			t++;
		} &&
		_signed_integer_like<iter_difference_t<T>>;

	template <class T>
	concept incrementable =
		weakly_incrementable<T> &&
		requires(T t) {
			{ t++ } -> same_as<T>;
		};

	template <class T>
	concept _can_reference = requires { typename T::_arbitrary; } || true;

	template <class I>
	concept input_or_output_iterator =
		weakly_incrementable<I> &&
		requires(I i) {
			{ *i } -> _can_reference;
		} &&
		(indirectly_readable<I> || requires(I i, iter_value_t<I> v) { *i = v; });

	template <class S, class I>
	inline constexpr bool disable_sized_sentinel_for = false;

	template <class S, class I>
	concept sentinel_for =
		semiregular<S> &&
		input_or_output_iterator<I> &&
		_weakly_equality_comparable_with<S, I>;

	template <class S, class I>
	concept sized_sentinel_for =
		sentinel_for<S, I> &&
		!disable_sized_sentinel_for<remove_cv_t<S>, remove_cv_t<I>> &&
		requires(const I &i, const S &s) {
			{ s - i } -> same_as<iter_difference_t<I>>;
			{ i - s } -> same_as<iter_difference_t<I>>;
		};

	namespace _detail {
		template <class I>
		struct _iter_concept_impl {
		};

		template <class I>
			requires requires { typename iterator_traits<I>::iterator_category; }
		struct _iter_concept_impl<I> {
			using type = typename iterator_traits<I>::iterator_category;
		};
		template <class I>
		using _iter_concept = typename _iter_concept_impl<I>::type;
	}

	template <class I>
	concept input_iterator =
		input_or_output_iterator<I> &&
		indirectly_readable<I> &&
		requires { typename _detail::_iter_concept<I>; } &&
		derived_from<_detail::_iter_concept<I>, input_iterator_tag>;

	template <class I, class T>
	concept output_iterator =
		input_or_output_iterator<I> &&
		indirectly_writable<I, T> &&
		requires(I i, T &&t) {
			*i++ = std::forward<T>(t);
		};

	template <class I>
	concept forward_iterator =
		input_iterator<I> &&
		derived_from<_detail::_iter_concept<I>, forward_iterator_tag> &&
		incrementable<I> &&
		sentinel_for<I, I>;

	template <class I>
	concept bidirectional_iterator =
		forward_iterator<I> &&
		derived_from<_detail::_iter_concept<I>, bidirectional_iterator_tag> &&
		requires(I i) {
			{ --i } -> same_as<I &>;
			{ i-- } -> same_as<I>;
		};

	template <class I>
	concept random_access_iterator =
		bidirectional_iterator<I> &&
		derived_from<_detail::_iter_concept<I>, random_access_iterator_tag> &&
		totally_ordered<I> &&
		sized_sentinel_for<I, I> &&
		requires(I i, const I j, const iter_difference_t<I> n) {
			{ i += n } -> same_as<I &>;
			{ j + n } -> same_as<I>;
			{ n + j } -> same_as<I>;
			{ i -= n } -> same_as<I &>;
			{ j - n } -> same_as<I>;
			{ j[n] } -> same_as<iter_reference_t<I>>;
		};

	template <class I>
	concept contiguous_iterator =
		random_access_iterator<I> &&
		derived_from<_detail::_iter_concept<I>, contiguous_iterator_tag> &&
		is_lvalue_reference_v<iter_reference_t<I>> &&
		same_as<iter_value_t<I>, remove_cvref_t<iter_reference_t<I>>> &&
		requires(const I &i) {
			{ to_address(i) } -> same_as<add_pointer_t<iter_reference_t<I>>>;
		};
}

#endif
