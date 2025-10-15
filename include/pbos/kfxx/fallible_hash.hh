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
		PBOS_FORCEINLINE option_t<size_t> operator()(signed char x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<unsigned char> {
		PBOS_FORCEINLINE option_t<size_t> operator()(unsigned char x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<char> {
		PBOS_FORCEINLINE option_t<size_t> operator()(char x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<short> {
		PBOS_FORCEINLINE option_t<size_t> operator()(short x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<unsigned short> {
		PBOS_FORCEINLINE option_t<size_t> operator()(unsigned short x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<int> {
		PBOS_FORCEINLINE option_t<size_t> operator()(int x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<unsigned int> {
		PBOS_FORCEINLINE option_t<size_t> operator()(unsigned int x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<long> {
		PBOS_FORCEINLINE option_t<size_t> operator()(long x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<unsigned long> {
		PBOS_FORCEINLINE option_t<size_t> operator()(unsigned long x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<long long> {
		PBOS_FORCEINLINE option_t<size_t> operator()(long long x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hasher<unsigned long long> {
		PBOS_FORCEINLINE option_t<size_t> operator()(unsigned long long x) const {
			return (size_t)+x;
		}
	};
}

#endif
