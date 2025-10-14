#ifndef _FREESTDC_CXX_UTILITY_INTEGER_SEQUENCE_
#define _FREESTDC_CXX_UTILITY_INTEGER_SEQUENCE_

#include <cstddef>

namespace std {
	template <typename T, T... Ints>
	struct integer_sequence {
		using value_type = T;
		static constexpr size_t size() noexcept {
			return sizeof...(Ints);
		}
	};

	template <size_t... Ints>
	using index_sequence = integer_sequence<size_t, Ints...>;

	template <typename T, T I, T N, T... integers>
	struct _make_integer_sequence_helper {
		using type = typename _make_integer_sequence_helper<T, I + 1, N, integers..., I>::type;
	};

	template <typename T, T N, T... integers>
	struct _make_integer_sequence_helper<T, N, N, integers...> {
		using type = integer_sequence<T, integers...>;
	};

	template <typename T, T N>
	using make_integer_sequence = typename _make_integer_sequence_helper<T, 0, N>::type;

	template <size_t N>
	using make_index_sequence = make_integer_sequence<size_t, N>;

	template <typename... T>
	using index_sequence_for = make_index_sequence<sizeof...(T)>;

#define __cpp_lib_integer_sequence 201304L
}

#endif
