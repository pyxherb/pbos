#ifndef _PBOS_KFXX_FALLIBLE_HASH_H_
#define _PBOS_KFXX_FALLIBLE_HASH_H_

#include "basedefs.hh"
#include "hash.hh"
#include "option.hh"

namespace kfxx {
	template <typename T>
	struct FallibleHash {
		static_assert(!std::is_same_v<T, T>, "Hasher not found");
	};

	template <>
	struct FallibleHash<signed char> {
		PBOS_FORCEINLINE Option<size_t> operator()(signed char x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHash<unsigned char> {
		PBOS_FORCEINLINE Option<size_t> operator()(unsigned char x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHash<char> {
		PBOS_FORCEINLINE Option<size_t> operator()(char x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHash<short> {
		PBOS_FORCEINLINE Option<size_t> operator()(short x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHash<unsigned short> {
		PBOS_FORCEINLINE Option<size_t> operator()(unsigned short x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHash<int> {
		PBOS_FORCEINLINE Option<size_t> operator()(int x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHash<unsigned int> {
		PBOS_FORCEINLINE Option<size_t> operator()(unsigned int x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHash<long> {
		PBOS_FORCEINLINE Option<size_t> operator()(long x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHash<unsigned long> {
		PBOS_FORCEINLINE Option<size_t> operator()(unsigned long x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHash<long long> {
		PBOS_FORCEINLINE Option<size_t> operator()(long long x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHash<unsigned long long> {
		PBOS_FORCEINLINE Option<size_t> operator()(unsigned long long x) const {
			return (size_t)+x;
		}
	};
}

#endif
