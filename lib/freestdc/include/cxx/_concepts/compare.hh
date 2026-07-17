#ifndef _FREESTDC_CXX_CONCEPTS_COMPARE_
#define _FREESTDC_CXX_CONCEPTS_COMPARE_

#include "common_ref.hh"
#include "boolean_testable.hh"
#include <compare>

namespace std {
	template <class T, class U>
	concept _weakly_equality_comparable_with =
		requires(const std::remove_reference_t<T> &t,
			const std::remove_reference_t<U> &u) {
			{ t == u } -> boolean_testable;
			{ t != u } -> boolean_testable;
			{ u == t } -> boolean_testable;
			{ u != t } -> boolean_testable;
		};

#if __cplusplus <= 202302L
	template <class T, class U>
	concept _comparison_common_type_with =
		std::common_reference_with<
			const std::remove_reference_t<T> &,
			const std::remove_reference_t<U> &>;
#else
	template <class T, class U, class C = std::common_reference_t<const T &, const U &>>
	concept _comparison_common_type_with_impl =
		std::same_as<std::common_reference_t<const T &, const U &>,
			std::common_reference_t<const U &, const T &>> &&
		requires {
			requires std::convertible_to<const T &, const C &> ||
						 std::convertible_to<T, const C &>;
			requires std::convertible_to<const U &, const C &> ||
						 std::convertible_to<U, const C &>;
		};
	template <class T, class U>
	concept _comparison_common_type_with =
		_comparison_common_type_with_impl<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
#endif

	template <class T>
	concept equality_comparable = _weakly_equality_comparable_with<T, T>;

	template <class T, class U>
	concept equality_comparable_with =
		std::equality_comparable<T> &&
		std::equality_comparable<U> &&
		_comparison_common_type_with<T, U> &&
		std::equality_comparable<
			std::common_reference_t<
				const std::remove_reference_t<T> &,
				const std::remove_reference_t<U> &>> &&
		_weakly_equality_comparable_with<T, U>;

	template <class T, class U>
	concept _partially_ordered_with =
		requires(const std::remove_reference_t<T> &t,
			const std::remove_reference_t<U> &u) {
			{ t < u } -> boolean_testable;
			{ t > u } -> boolean_testable;
			{ t <= u } -> boolean_testable;
			{ t >= u } -> boolean_testable;
			{ u < t } -> boolean_testable;
			{ u > t } -> boolean_testable;
			{ u <= t } -> boolean_testable;
			{ u >= t } -> boolean_testable;
		};

	template <class T>
	concept totally_ordered =
		std::equality_comparable<T> && _partially_ordered_with<T, T>;

	template <class T, class U>
	concept totally_ordered_with =
		std::totally_ordered<T> &&
		std::totally_ordered<U> &&
		std::equality_comparable_with<T, U> &&
		std::totally_ordered<
			std::common_reference_t<
				const std::remove_reference_t<T> &,
				const std::remove_reference_t<U> &>> &&
		_partially_ordered_with<T, U>;

	template <class T, class Cat>
	concept _compare_as =
		std::same_as<std::common_comparison_category_t<T, Cat>, Cat>;

	template <class T, class Cat = std::partial_ordering>
	concept three_way_comparable =
		_weakly_equality_comparable_with<T, T> &&
		_partially_ordered_with<T, T> &&
		requires(const std::remove_reference_t<T> &a,
			const std::remove_reference_t<T> &b) {
			{ a <=> b } -> _compare_as<Cat>;
		};

	template <class T, class U, class Cat = std::partial_ordering>
	concept three_way_comparable_with =
		std::three_way_comparable<T, Cat> &&
		std::three_way_comparable<U, Cat> &&
		_comparison_common_type_with<T, U> &&
		std::three_way_comparable<
			std::common_reference_t<
				const std::remove_reference_t<T> &,
				const std::remove_reference_t<U> &>,
			Cat> &&
		_weakly_equality_comparable_with<T, U> &&
		_partially_ordered_with<T, U> &&
		requires(const std::remove_reference_t<T> &t,
			const std::remove_reference_t<U> &u) {
			{ t <=> u } -> _compare_as<Cat>;
			{ u <=> t } -> _compare_as<Cat>;
		};
}

#endif
