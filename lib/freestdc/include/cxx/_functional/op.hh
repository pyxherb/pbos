#ifndef _FREESTDC_CXX_FUNCTIONAL_OP_
#define _FREESTDC_CXX_FUNCTIONAL_OP_

#include <_utility/forward.hh>
#include <cxx_basedefs.hh>
#include "../_concepts/compare.hh"

namespace std {
	template <typename T>
	struct plus {
		FREESTDC_FORCEINLINE constexpr T operator()(const T &x, const T &y) const {
			return x + y;
		}
	};

	template <>
	struct plus<void> {
		template <typename T, typename U>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t, U &&u) const
			-> decltype(std::forward<T>(t) + std::forward<U>(u)) {
			return static_cast<T &&>(t) + static_cast<U &&>(u);
		}

		using is_transparent = int;
	};

	template <typename T = void>
	struct minus {
		FREESTDC_FORCEINLINE constexpr T operator()(const T &x, const T &y) const {
			return x - y;
		}
	};

	template <>
	struct minus<void> {
		template <typename T, typename U>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t, U &&u) const
			-> decltype(std::forward<T>(t) - std::forward<U>(u)) {
			return static_cast<T &&>(t) - static_cast<U &&>(u);
		}

		using is_transparent = int;
	};

	template <typename T = void>
	struct multiplies {
		FREESTDC_FORCEINLINE constexpr T operator()(const T &x, const T &y) const {
			return x * y;
		}
	};

	template <>
	struct multiplies<void> {
		template <typename T, typename U>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t, U &&u) const
			-> decltype(std::forward<T>(t) * std::forward<U>(u)) {
			return static_cast<T &&>(t) * static_cast<U &&>(u);
		}

		using is_transparent = int;
	};

	template <typename T = void>
	struct divides {
		FREESTDC_FORCEINLINE constexpr T operator()(const T &x, const T &y) const {
			return x / y;
		}
	};

	template <>
	struct divides<void> {
		template <typename T, typename U>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t, U &&u) const
			-> decltype(std::forward<T>(t) / std::forward<U>(u)) {
			return static_cast<T &&>(t) / static_cast<U &&>(u);
		}

		using is_transparent = int;
	};

	template <typename T = void>
	struct modulus {
		FREESTDC_FORCEINLINE constexpr T operator()(const T &x, const T &y) const {
			return x % y;
		}
	};

	template <>
	struct modulus<void> {
		template <typename T, typename U>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t, U &&u) const
			-> decltype(std::forward<T>(t) % std::forward<U>(u)) {
			return static_cast<T &&>(t) % static_cast<U &&>(u);
		}

		using is_transparent = int;
	};

	template <typename T = void>
	struct negate {
		FREESTDC_FORCEINLINE constexpr T operator()(const T &x) const {
			return -x;
		}
	};

	template <>
	struct negate<void> {
		template <typename T>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t) const
			-> decltype(-std::forward<T>(t)) {
			return -static_cast<T &&>(t);
		}

		using is_transparent = int;
	};

	template <typename T = void>
	struct equal_to {
		FREESTDC_FORCEINLINE constexpr bool operator()(const T &x, const T &y) const {
			return x == y;
		}
	};

	template <>
	struct equal_to<void> {
		template <typename T, typename U>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t, U &&u) const
			-> decltype(std::forward<T>(t) == std::forward<U>(u)) {
			return static_cast<T &&>(t) == static_cast<U &&>(u);
		}

		using is_transparent = int;
	};

	template <typename T = void>
	struct not_equal_to {
		FREESTDC_FORCEINLINE constexpr bool operator()(const T &x, const T &y) const {
			return x != y;
		}
	};

	template <>
	struct not_equal_to<void> {
		template <typename T, typename U>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t, U &&u) const
			-> decltype(std::forward<T>(t) != std::forward<U>(u)) {
			return static_cast<T &&>(t) != static_cast<U &&>(u);
		}

		using is_transparent = int;
	};

	template <typename T = void>
	struct greater {
		FREESTDC_FORCEINLINE constexpr bool operator()(const T &x, const T &y) const {
			return x > y;
		}
	};

	template <>
	struct greater<void> {
		template <typename T, typename U>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t, U &&u) const
			-> decltype(std::forward<T>(t) > std::forward<U>(u)) {
			return static_cast<T &&>(t) > static_cast<U &&>(u);
		}

		using is_transparent = int;
	};

	template <typename T = void>
	struct less {
		FREESTDC_FORCEINLINE constexpr bool operator()(const T &x, const T &y) const {
			return x < y;
		}
	};

	template <>
	struct less<void> {
		template <typename T, typename U>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t, U &&u) const
			-> decltype(std::forward<T>(t) < std::forward<U>(u)) {
			return static_cast<T &&>(t) < static_cast<U &&>(u);
		}

		using is_transparent = int;
	};

	template <typename T = void>
	struct greater_equal {
		FREESTDC_FORCEINLINE constexpr bool operator()(const T &x, const T &y) const {
			return x >= y;
		}
	};

	template <>
	struct greater_equal<void> {
		template <typename T, typename U>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t, U &&u) const
			-> decltype(std::forward<T>(t) >= std::forward<U>(u)) {
			return static_cast<T &&>(t) >= static_cast<U &&>(u);
		}

		using is_transparent = int;
	};

	template <typename T = void>
	struct less_equal {
		FREESTDC_FORCEINLINE constexpr bool operator()(const T &x, const T &y) const {
			return x <= y;
		}
	};

	template <>
	struct less_equal<void> {
		template <typename T, typename U>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t, U &&u) const
			-> decltype(std::forward<T>(t) <= std::forward<U>(u)) {
			return static_cast<T &&>(t) <= static_cast<U &&>(u);
		}

		using is_transparent = int;
	};

	namespace ranges {
		struct less {
			template <typename _Tp, typename _Up>
				requires totally_ordered_with<_Tp, _Up>
			constexpr bool operator()(_Tp &&__t, _Up &&__u) const
				noexcept(noexcept(static_cast<bool>(std::forward<_Tp>(__t) < std::forward<_Up>(__u)))) {
				return std::forward<_Tp>(__t) < std::forward<_Up>(__u);
			}
		};

		struct greater {
			template <typename _Tp, typename _Up>
				requires totally_ordered_with<_Tp, _Up>
			constexpr bool operator()(_Tp &&__t, _Up &&__u) const
				noexcept(noexcept(static_cast<bool>(std::forward<_Tp>(__t) > std::forward<_Up>(__u)))) {
				return std::forward<_Tp>(__t) > std::forward<_Up>(__u);
			}
		};

		struct less_equal {
			template <typename _Tp, typename _Up>
				requires totally_ordered_with<_Tp, _Up>
			constexpr bool operator()(_Tp &&__t, _Up &&__u) const
				noexcept(noexcept(static_cast<bool>(std::forward<_Tp>(__t) <= std::forward<_Up>(__u)))) {
				return std::forward<_Tp>(__t) <= std::forward<_Up>(__u);
			}
		};

		struct greater_equal {
			template <typename _Tp, typename _Up>
				requires totally_ordered_with<_Tp, _Up>
			constexpr bool operator()(_Tp &&__t, _Up &&__u) const
				noexcept(noexcept(static_cast<bool>(std::forward<_Tp>(__t) >= std::forward<_Up>(__u)))) {
				return std::forward<_Tp>(__t) >= std::forward<_Up>(__u);
			}
		};

		struct equal_to {
			template <typename _Tp, typename _Up>
				requires equality_comparable_with<_Tp, _Up>
			constexpr bool operator()(_Tp &&__t, _Up &&__u) const
				noexcept(noexcept(static_cast<bool>(std::forward<_Tp>(__t) == std::forward<_Up>(__u)))) {
				return std::forward<_Tp>(__t) == std::forward<_Up>(__u);
			}
		};

		struct not_equal_to {
			template <typename _Tp, typename _Up>
				requires equality_comparable_with<_Tp, _Up>
			constexpr bool operator()(_Tp &&__t, _Up &&__u) const
				noexcept(noexcept(static_cast<bool>(std::forward<_Tp>(__t) != std::forward<_Up>(__u)))) {
				return std::forward<_Tp>(__t) != std::forward<_Up>(__u);
			}
		};
	}

	template <typename T = void>
	struct logical_and {
		FREESTDC_FORCEINLINE constexpr bool operator()(const T &x, const T &y) const {
			return x && y;
		}
	};

	template <>
	struct logical_and<void> {
		template <typename T, typename U>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t, U &&u) const
			-> decltype(std::forward<T>(t) && std::forward<U>(u)) {
			return static_cast<T &&>(t) && static_cast<U &&>(u);
		}

		using is_transparent = int;
	};

	template <typename T = void>
	struct logical_or {
		FREESTDC_FORCEINLINE constexpr bool operator()(const T &x, const T &y) const {
			return x || y;
		}
	};

	template <>
	struct logical_or<void> {
		template <typename T, typename U>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t, U &&u) const
			-> decltype(std::forward<T>(t) || std::forward<U>(u)) {
			return static_cast<T &&>(t) || static_cast<U &&>(u);
		}

		using is_transparent = int;
	};

	template <typename T = void>
	struct logical_not {
		FREESTDC_FORCEINLINE constexpr bool operator()(const T &x) const {
			return !x;
		}
	};

	template <>
	struct logical_not<void> {
		template <typename T>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t) const
			-> decltype(!std::forward<T>(t)) {
			return !static_cast<T &&>(t);
		}

		using is_transparent = int;
	};

	template <typename T = void>
	struct bit_and {
		FREESTDC_FORCEINLINE constexpr T operator()(const T &x, const T &y) const {
			return x & y;
		}
	};

	template <>
	struct bit_and<void> {
		template <typename T, typename U>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t, U &&u) const
			-> decltype(std::forward<T>(t) & std::forward<U>(u)) {
			return static_cast<T &&>(t) & static_cast<U &&>(u);
		}

		using is_transparent = int;
	};

	template <typename T = void>
	struct bit_or {
		FREESTDC_FORCEINLINE constexpr T operator()(const T &x, const T &y) const {
			return x | y;
		}
	};

	template <>
	struct bit_or<void> {
		template <typename T, typename U>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t, U &&u) const
			-> decltype(std::forward<T>(t) | std::forward<U>(u)) {
			return static_cast<T &&>(t) | static_cast<U &&>(u);
		}

		using is_transparent = int;
	};

	template <typename T = void>
	struct bit_xor {
		FREESTDC_FORCEINLINE constexpr T operator()(const T &x, const T &y) const {
			return x ^ y;
		}
	};

	template <>
	struct bit_xor<void> {
		template <typename T, typename U>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t, U &&u) const
			-> decltype(std::forward<T>(t) ^ std::forward<U>(u)) {
			return static_cast<T &&>(t) ^ static_cast<U &&>(u);
		}

		using is_transparent = int;
	};

	template <typename T = void>
	struct bit_not {
		FREESTDC_FORCEINLINE constexpr T operator()(const T &x) const {
			return ~x;
		}
	};

	template <>
	struct bit_not<void> {
		template <typename T>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t) const
			-> decltype(~std::forward<T>(t)) {
			return ~static_cast<T &&>(t);
		}

		using is_transparent = int;
	};

#if __cplusplus >= 202002L
	struct compare_three_way {
		template <typename T, typename U>
		FREESTDC_FORCEINLINE constexpr auto operator()(T &&t, U &&u) const {
			return static_cast<T &&>(t) <=> static_cast<U &&>(u);
		}

		using is_transparent = int;
	};
#endif
}

#endif
