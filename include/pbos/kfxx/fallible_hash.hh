#ifndef _PBOS_KFXX_FALLIBLE_HASH_H_
#define _PBOS_KFXX_FALLIBLE_HASH_H_

#include "basedefs.hh"
#include "hash.hh"
#include "option.hh"

namespace kfxx {
	template <typename T>
	struct fallible_hash {
		static_assert(!std::is_same_v<T, T>, "Hasher not found");
	};

	template <>
	struct fallible_hash<signed char> {
		PBOS_FORCEINLINE option_t<size_t> operator()(signed char x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hash<unsigned char> {
		PBOS_FORCEINLINE option_t<size_t> operator()(unsigned char x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hash<char> {
		PBOS_FORCEINLINE option_t<size_t> operator()(char x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hash<short> {
		PBOS_FORCEINLINE option_t<size_t> operator()(short x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hash<unsigned short> {
		PBOS_FORCEINLINE option_t<size_t> operator()(unsigned short x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hash<int> {
		PBOS_FORCEINLINE option_t<size_t> operator()(int x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hash<unsigned int> {
		PBOS_FORCEINLINE option_t<size_t> operator()(unsigned int x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hash<long> {
		PBOS_FORCEINLINE option_t<size_t> operator()(long x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hash<unsigned long> {
		PBOS_FORCEINLINE option_t<size_t> operator()(unsigned long x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hash<long long> {
		PBOS_FORCEINLINE option_t<size_t> operator()(long long x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct fallible_hash<unsigned long long> {
		PBOS_FORCEINLINE option_t<size_t> operator()(unsigned long long x) const {
			return (size_t)+x;
		}
	};
}

#endif
