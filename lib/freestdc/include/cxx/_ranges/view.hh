#ifndef _FREESTDC_CXX_RANGES_VIEW_
#define _FREESTDC_CXX_RANGES_VIEW_

#include "../_iterator/iter_ops.hh"
#include "concepts.hh"

namespace std {
	namespace ranges {
		struct view_base {};

		template <class T>
		constexpr bool enable_view = derived_from<T, view_base>;

		template <class T>
		concept view = range<T> && movable<T> && enable_view<T>;

		template <class T>
		concept viewable_range = range<T> &&
								 (borrowed_range<T> || view<remove_cvref_t<T>>);

		template <class D>
			requires is_class_v<D> && same_as<D, remove_cv_t<D>>
		class view_interface {
		public:
			constexpr D &derived() noexcept { return static_cast<D &>(*this); }
			constexpr const D &derived() const noexcept { return static_cast<const D &>(*this); }

			constexpr bool empty()
				requires forward_range<D>
			{
				if constexpr (sized_range<D>)
					return derived().size() == 0;
				else
					return ranges::begin(derived()) == ranges::end(derived());
			}

			constexpr bool empty() const
				requires forward_range<const D>
			{
				if constexpr (sized_range<const D>)
					return derived().size() == 0;
				else
					return ranges::begin(derived()) == ranges::end(derived());
			}

			constexpr explicit operator bool()
				requires requires { derived().empty(); }
			{
				return !derived().empty();
			}

			constexpr explicit operator bool() const
				requires requires { derived().empty(); }
			{
				return !derived().empty();
			}

			constexpr auto data()
				requires contiguous_range<D>
			{
				return ranges::begin(derived());
			}

			constexpr auto data() const
				requires contiguous_range<const D>
			{
				return ranges::begin(derived());
			}

			constexpr auto size()
				requires forward_range<D> && sized_sentinel_for<sentinel_t<D>, iterator_t<D>>
			{
				return ranges::end(derived()) - ranges::begin(derived());
			}

			constexpr auto size() const
				requires forward_range<const D> && sized_sentinel_for<sentinel_t<const D>, iterator_t<const D>>
			{
				return ranges::end(derived()) - ranges::begin(derived());
			}

			constexpr decltype(auto) front()
				requires forward_range<D>
			{
				return *ranges::begin(derived());
			}

			constexpr decltype(auto) front() const
				requires forward_range<const D>
			{
				return *ranges::begin(derived());
			}

			constexpr decltype(auto) back()
				requires bidirectional_range<D> && common_range<D>
			{
				return *ranges::prev(ranges::end(derived()));
			}

			constexpr decltype(auto) back() const
				requires bidirectional_range<const D> && common_range<const D>
			{
				return *ranges::prev(ranges::end(derived()));
			}

			template <class T = D>
				requires random_access_range<D>
			constexpr decltype(auto) operator[](range_difference_t<T> n) {
				return ranges::begin(derived())[n];
			}

			template <class T = const D>
				requires random_access_range<const D>
			constexpr decltype(auto) operator[](range_difference_t<T> n) const {
				return ranges::begin(derived())[n];
			}
		};

		enum class subrange_kind : bool {
			unsized,
			sized
		};

		template <class T, class U>
		concept _different_from = !std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

		template <
			input_or_output_iterator I,
			sentinel_for<I> S = I,
			subrange_kind K = sized_sentinel_for<S, I> ? subrange_kind::sized
													   : subrange_kind::unsized>
			requires(K == subrange_kind::sized || !sized_sentinel_for<S, I>)
		class subrange : public view_interface<subrange<I, S, K>> {
		private:
			static constexpr bool StoreSize = K == subrange_kind::sized && !sized_sentinel_for<S, I>;

			I begin_ = I();
			S end_ = S();

			[[no_unique_address]] conditional_t<StoreSize, iter_difference_t<I>, int> size_ = 0;

		public:
			subrange() = default;

			template <class I2, class S2>
				requires convertible_to<I2, I> && convertible_to<S2, S>
			constexpr subrange(I2 i, S2 s)
				: begin_(move(i)), end_(move(s)) {}

			template <class I2, class S2>
				requires convertible_to<I2, I> && convertible_to<S2, S>
											  constexpr subrange(I2 i, S2 s, iter_difference_t<I> n)
							 requires(K == subrange_kind::sized)
				: begin_(move(i)), end_(move(s)), size_(n) {}

			template <_different_from<subrange> R>
				requires borrowed_range<R> &&
						 convertible_to<iterator_t<R>, I> &&
						 convertible_to<sentinel_t<R>, S>
			constexpr subrange(R &&r)
				: subrange(ranges::begin(r), ranges::end(r)) {}

			template <_different_from<subrange> R>
				requires borrowed_range<R> &&
						 convertible_to<iterator_t<R>, I> &&
						 convertible_to<sentinel_t<R>, S>
						 constexpr subrange(R &&r, iter_difference_t<I> n)
							 requires(K == subrange_kind::sized)
				: subrange(ranges::begin(r), ranges::end(r), n) {}

#if __cplusplus >= 202302L

			template <class PairLike>
				requires(!same_as<PairLike, subrange>) &&
						pair_like<PairLike> &&
						convertible_to<tuple_element_t<0, remove_cvref_t<PairLike>>, I> &&
						convertible_to<tuple_element_t<1, remove_cvref_t<PairLike>>, S>
			constexpr subrange(PairLike &&p)
				: subrange(get<0>(forward<PairLike>(p)), get<1>(forward<PairLike>(p))) {}

			template <class R>
				requires(!same_as<remove_cvref_t<R>, subrange>) &&
						ranges::input_range<R> &&
						convertible_to<ranges::iterator_t<R>, I> &&
						convertible_to<ranges::sentinel_t<R>, S>
			constexpr subrange(from_range_t, R &&r)
				: subrange(ranges::begin(r), ranges::end(r)) {}
#endif

			constexpr I begin() const { return begin_; }
			constexpr S end() const { return end_; }

			constexpr iter_difference_t<I> size() const
				requires(K == subrange_kind::sized)
			{
				if constexpr (sized_sentinel_for<S, I>)
					return end_ - begin_;
				else
					return size_;
			}

			constexpr subrange &advance(iter_difference_t<I> n) {
				if constexpr (bidirectional_iterator<I>)
					if (n < 0) {
						ranges::advance(begin_, n);
						return *this;
					}
				size_ -= n - ranges::advance(begin_, n, end_);
				return *this;
			}

			constexpr subrange next(iter_difference_t<I> n = 1) const &
				requires forward_iterator<I>
			{
				auto tmp = *this;
				tmp.advance(n);
				return tmp;
			}
			constexpr subrange next(iter_difference_t<I> n = 1) && {
				advance(n);
				return move(*this);
			}

			constexpr subrange prev(iter_difference_t<I> n = 1) const
				requires bidirectional_iterator<I>
			{
				auto tmp = *this;
				tmp.advance(-n);
				return tmp;
			}
		};

		template <class I, class S>
		subrange(I, S) -> subrange<I, S>;

		template <class I, class S>
		subrange(I, S, iter_difference_t<I>) -> subrange<I, S, subrange_kind::sized>;

		template <class R>
		subrange(R &&) -> subrange<iterator_t<R>, sentinel_t<R>, (sized_range<R> || sized_sentinel_for<sentinel_t<R>, iterator_t<R>>) ? subrange_kind::sized : subrange_kind::unsized>;

		template <class R>
		subrange(R &&, iter_difference_t<iterator_t<R>>) -> subrange<iterator_t<R>, sentinel_t<R>, subrange_kind::sized>;

#if __cplusplus >= 202302L
		template <class PairLike>
		subrange(PairLike &&) -> subrange<
			tuple_element_t<0, remove_cvref_t<PairLike>>,
			tuple_element_t<1, remove_cvref_t<PairLike>>>;
#endif

		template <class I, class S, subrange_kind K>
		inline constexpr bool enable_borrowed_range<subrange<I, S, K>> = true;

	}
}

#endif
