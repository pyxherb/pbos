#ifndef _PBOS_KFXX_FALLIBLE_HASH_H_
#define _PBOS_KFXX_FALLIBLE_HASH_H_

#include "basedefs.hh"
#include "option.hh"

namespace kfxx {
	template <typename T>
	struct fallible_hasher {
		static_assert(!std::is_same_v<T, T>, "Hasher was not found");
	};

	template <>
	struct fallible_hasher<signed char> {
		PB_FORCEINLINE option_t<size_t> operator()(signed char x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<unsigned char> {
		PB_FORCEINLINE option_t<size_t> operator()(unsigned char x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<char> {
		PB_FORCEINLINE option_t<size_t> operator()(char x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<short> {
		PB_FORCEINLINE option_t<size_t> operator()(short x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<unsigned short> {
		PB_FORCEINLINE option_t<size_t> operator()(unsigned short x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<int> {
		PB_FORCEINLINE option_t<size_t> operator()(int x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<unsigned int> {
		PB_FORCEINLINE option_t<size_t> operator()(unsigned int x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<long> {
		PB_FORCEINLINE option_t<size_t> operator()(long x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<unsigned long> {
		PB_FORCEINLINE option_t<size_t> operator()(unsigned long x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<long long> {
		PB_FORCEINLINE option_t<size_t> operator()(long long x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<unsigned long long> {
		PB_FORCEINLINE option_t<size_t> operator()(unsigned long long x) const {
			return (size_t)+x;
		}
	};
}

#endif
