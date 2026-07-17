#ifndef _FREESTDC_CXX_RANGES_CONCEPTS_
#define _FREESTDC_CXX_RANGES_CONCEPTS_

#include "../_concepts/derived_from.hh"
#include "../_iterator/concepts.hh"
#include "access.hh"

namespace std {
	namespace ranges {
		using std::iter_difference_t;
		using std::iter_reference_t;
		using std::iter_value_t;

		template <class R>
		using iterator_t = decltype(ranges::begin(std::declval<R &>()));

		template <class R>
		using sentinel_t = decltype(ranges::end(std::declval<R &>()));

		template <class R>
		using range_difference_t = iter_difference_t<iterator_t<R>>;

		template <class R>
		using range_value_t = iter_value_t<iterator_t<R>>;

		template <class R>
		using range_reference_t = iter_reference_t<iterator_t<R>>;

		template <class R>
		using range_rvalue_reference_t = decltype(iter_move(std::declval<iterator_t<R> &>()));

		template <class T>
		constexpr bool enable_borrowed_range = false;

		template <class T>
		concept range = requires(T &t) {
			ranges::begin(t);
			ranges::end(t);
		};

		template <class T>
		concept borrowed_range = range<T> &&
								 (is_lvalue_reference_v<T> || enable_borrowed_range<remove_cvref_t<T>>);

		template <class T>
		concept sized_range = range<T> && requires(T &t) {
			ranges::size(t);
		};

#if __cplusplus >= 202400L
		template <class T>
		concept approximately_sized_range = range<T> && requires(const T &t) {
			{ ranges::size(t) } -> convertible_to<size_t>;
		};
#endif

		template <class T>
		concept input_range = range<T> && input_iterator<iterator_t<T>>;

		template <class T, class U>
		concept output_range = range<T> && output_iterator<iterator_t<T>, U>;

		template <class T>
		concept forward_range = input_range<T> && forward_iterator<iterator_t<T>>;

		template <class T>
		concept bidirectional_range = forward_range<T> && bidirectional_iterator<iterator_t<T>>;

		template <class T>
		concept random_access_range = bidirectional_range<T> && random_access_iterator<iterator_t<T>>;

		template <class T>
		concept contiguous_range =
			random_access_range<T> &&
			contiguous_iterator<iterator_t<T>> &&
			requires(T &t) {
				{ ranges::data(t) } -> same_as<add_pointer_t<range_reference_t<T>>>;
			};

		template <class T>
		concept common_range = range<T> && same_as<iterator_t<T>, sentinel_t<T>>;

#if __cplusplus >= 202302L
		template <class T>
		concept constant_range =
			input_range<T> &&
			same_as<iter_const_reference_t<iterator_t<T>>, iter_reference_t<iterator_t<T>>>;
#endif
	}
}

#endif
