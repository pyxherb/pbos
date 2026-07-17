#ifndef _FREESTDC_CXX_ITERATOR_ITER_OPS_
#define _FREESTDC_CXX_ITERATOR_ITER_OPS_

#include "../_ranges/concepts.hh"
#include "concepts.hh"

namespace std {
	template <class It, class Distance>
	constexpr void advance(It &it, Distance n) {
		using category = typename std::iterator_traits<It>::iterator_category;
		static_assert(std::is_base_of_v<std::input_iterator_tag, category>);

		auto dist = typename std::iterator_traits<It>::difference_type(n);
		if constexpr (std::is_base_of_v<std::random_access_iterator_tag, category>)
			it += dist;
		else {
			while (dist > 0) {
				--dist;
				++it;
			}
			if constexpr (std::is_base_of_v<std::bidirectional_iterator_tag, category>)
				while (dist < 0) {
					++dist;
					--it;
				}
		}
	}

	template <class It>
	constexpr typename std::iterator_traits<It>::difference_type distance(It first, It last) {
		using category = typename std::iterator_traits<It>::iterator_category;
		static_assert(std::is_base_of_v<std::input_iterator_tag, category>);

		if constexpr (std::is_base_of_v<std::random_access_iterator_tag, category>)
			return last - first;
		else {
			typename std::iterator_traits<It>::difference_type result = 0;
			while (first != last) {
				++first;
				++result;
			}
			return result;
		}
	}

	template <class InputIt>
	constexpr InputIt next(InputIt it, typename std::iterator_traits<InputIt>::difference_type n = 1) {
		std::advance(it, n);
		return it;
	}

	template <class BidirIt>
	constexpr BidirIt prev(BidirIt it, typename std::iterator_traits<BidirIt>::difference_type n = 1) {
		std::advance(it, -n);
		return it;
	}

	namespace ranges {
		struct advance_fn {
			template <std::input_or_output_iterator I>
			constexpr void operator()(I &i, std::iter_difference_t<I> n) const {
				if constexpr (std::random_access_iterator<I>)
					i += n;
				else {
					while (n > 0) {
						--n;
						++i;
					}

					if constexpr (std::bidirectional_iterator<I>) {
						while (n < 0) {
							++n;
							--i;
						}
					}
				}
			}

			template <std::input_or_output_iterator I, std::sentinel_for<I> S>
			constexpr void operator()(I &i, S bound) const {
				if constexpr (std::assignable_from<I &, S>)
					i = std::move(bound);
				else if constexpr (std::sized_sentinel_for<S, I>)
					(*this)(i, bound - i);
				else
					while (i != bound)
						++i;
			}

			template <std::input_or_output_iterator I, std::sentinel_for<I> S>
			constexpr std::iter_difference_t<I>
			operator()(I &i, std::iter_difference_t<I> n, S bound) const {
				if constexpr (std::sized_sentinel_for<S, I>) {
					auto abs = [](const std::iter_difference_t<I> x) { return x < 0 ? -x : x; };

					if (const auto dist = abs(n) - abs(bound - i); dist < 0) {
						(*this)(i, bound);
						return -dist;
					}

					(*this)(i, n);
					return 0;
				} else {
					while (n > 0 && i != bound) {
						--n;
						++i;
					}

					if constexpr (std::bidirectional_iterator<I>) {
						while (n < 0 && i != bound) {
							++n;
							--i;
						}
					}

					return n;
				}
			}
		};

		inline constexpr auto advance = advance_fn();

		struct distance_fn {
			template <class I, std::sentinel_for<I> S>
				requires(!std::sized_sentinel_for<S, I>)
			constexpr std::iter_difference_t<I> operator()(I first, S last) const {
				std::iter_difference_t<I> result = 0;
				while (first != last) {
					++first;
					++result;
				}
				return result;
			}

			template <class I, std::sized_sentinel_for<std::decay<I>> S>
			constexpr std::iter_difference_t<I> operator()(const I &first, S last) const {
				return last - first;
			}

			template <ranges::range R>
			constexpr ranges::range_difference_t<R> operator()(R &&r) const {
				if constexpr (ranges::sized_range<std::remove_cvref_t<R>>)
					return static_cast<ranges::range_difference_t<R>>(ranges::size(r));
				else
					return (*this)(ranges::begin(r), ranges::end(r));
			}
		};

		inline constexpr auto distance = distance_fn{};

		struct next_fn {
			template <std::input_or_output_iterator I>
			constexpr I operator()(I i) const {
				++i;
				return i;
			}

			template <std::input_or_output_iterator I>
			constexpr I operator()(I i, std::iter_difference_t<I> n) const {
				ranges::advance(i, n);
				return i;
			}

			template <std::input_or_output_iterator I, std::sentinel_for<I> S>
			constexpr I operator()(I i, S bound) const {
				ranges::advance(i, bound);
				return i;
			}

			template <std::input_or_output_iterator I, std::sentinel_for<I> S>
			constexpr I operator()(I i, std::iter_difference_t<I> n, S bound) const {
				ranges::advance(i, n, bound);
				return i;
			}
		};

		inline constexpr auto next = next_fn();

		struct prev_fn {
			template <std::bidirectional_iterator I>
			constexpr I operator()(I i) const {
				--i;
				return i;
			}

			template <std::bidirectional_iterator I>
			constexpr I operator()(I i, std::iter_difference_t<I> n) const {
				ranges::advance(i, -n);
				return i;
			}

			template <std::bidirectional_iterator I>
			constexpr I operator()(I i, std::iter_difference_t<I> n, I bound) const {
				ranges::advance(i, -n, bound);
				return i;
			}
		};

		inline constexpr auto prev = prev_fn();
	}
}

#endif
