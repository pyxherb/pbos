#ifndef _FREESTDC_CXX_UTILITY_TUPLE_
#define _FREESTDC_CXX_UTILITY_TUPLE_

#include <_type_traits/conditional.hh>
#include <_type_traits/enable_if.hh>
#include <_type_traits/is_empty.hh>
#include <_type_traits/is_final.hh>
#include <cstddef>
#include "forward.hh"

namespace std {
	template <typename T>
	struct _is_empty_non_tuple : is_empty<T> {};
	template <typename T>
	using _is_empty_non_final = typename conditional<is_final_v<T>, false_type, _is_empty_non_tuple<T>>::type;

	template <size_t Index, typename Head, bool = _is_empty_non_final<Head>::value>
	struct _head_base;

	// For inheritable types.
	template <size_t Index, typename Head>
	struct _head_base<Index, Head, true> : public Head {
		constexpr _head_base() : Head() {}
		constexpr _head_base(const Head &h) : Head(h) {}
		template <typename U>
		constexpr _head_base(U &&h) : Head(forward<U>(h)) {}
		constexpr _head_base(const _head_base &) = default;
		constexpr _head_base(_head_base &&) = default;

		static constexpr Head &_get_head(_head_base &b) noexcept { return b; }
		static constexpr const Head &_get_head(const _head_base &b) noexcept { return b; }
	};

	template <size_t Index, typename Head>
	struct _head_base<Index, Head, false> {
		Head _head;

		constexpr _head_base() : _head() {}
		constexpr _head_base(const Head &h) : _head(h) {}
		template <typename U>
		constexpr _head_base(U &&h) : _head(std::forward<U>(h)) {}
		constexpr _head_base(const _head_base &) = default;
		constexpr _head_base(_head_base &&) = default;

		static constexpr Head &_get_head(_head_base &b) noexcept { return b._head; }
		static constexpr const Head &_get_head(const _head_base &b) noexcept { return b._head; }
	};

	template <size_t Index, typename Head, typename... Tail>
	struct _tuple_impl
		: public _tuple_impl<Index, Tail...>,
		  private _head_base<Index, Head> {
		using _inherited_t = _tuple_impl<Index + 1, Tail...>;
		using _base_t = _head_base<Index, Head>;

		constexpr _tuple_impl() : _inherited_t(), _base_t() {}

		explicit constexpr _tuple_impl(const Head &head, const Tail &...tail)
			: _inherited_t(tail...), _base_t(head) {}

		template <typename Head2,
			typename... Tail2,
			typename Enable = typename enable_if<sizeof...(Tail) == sizeof...(Tail2)>::type>
		explicit constexpr _tuple_impl(Head2 &&head, Tail2 &&...tail)
			: _inherited_t(std::forward<Tail2>(tail)...),
			  _base_t(std::forward<Head2>(head)) {}
	};

	template <size_t Index, typename Head>
	struct _tuple_impl<Index, Head> : private _head_base<Index, Head> {
		using _inherited_t = _tuple_impl<Index + 1, Head>;
		using _base_t = _head_base<Index, Head>;

		constexpr _tuple_impl() : _base_t() {}

		explicit constexpr _tuple_impl(const Head &head) : _base_t(head) {}

		template <typename U>
		explicit constexpr _tuple_impl(U &&__head)
			: _base_t(std::forward<U>(__head)) {}
	};

	template <typename... Ts>
	class tuple : public _tuple_impl<0, Ts...> {
		// TODO: Implement others...
	};
}

#endif
