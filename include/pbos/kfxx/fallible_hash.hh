#ifndef _PBOS_KFXX_FALLIBLE_HASH_H_
#define _PBOS_KFXX_FALLIBLE_HASH_H_

#include "basedefs.hh"
#include "option.hh"

namespace kfxx {
	template <typename T>
	struct FallibleHasher {
		static_assert(!std::is_same_v<T, T>, "Hasher was not found");
	};

	template <>
	struct FallibleHasher<signed char> {
		PB_FORCEINLINE Option<size_t> operator()(signed char x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHasher<unsigned char> {
		PB_FORCEINLINE Option<size_t> operator()(unsigned char x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHasher<char> {
		PB_FORCEINLINE Option<size_t> operator()(char x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHasher<short> {
		PB_FORCEINLINE Option<size_t> operator()(short x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHasher<unsigned short> {
		PB_FORCEINLINE Option<size_t> operator()(unsigned short x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHasher<int> {
		PB_FORCEINLINE Option<size_t> operator()(int x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHasher<unsigned int> {
		PB_FORCEINLINE Option<size_t> operator()(unsigned int x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHasher<long> {
		PB_FORCEINLINE Option<size_t> operator()(long x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHasher<unsigned long> {
		PB_FORCEINLINE Option<size_t> operator()(unsigned long x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHasher<long long> {
		PB_FORCEINLINE Option<size_t> operator()(long long x) const {
			return (size_t)+x;
		}
	};

	template <>
	struct FallibleHasher<unsigned long long> {
		PB_FORCEINLINE Option<size_t> operator()(unsigned long long x) const {
			return (size_t)+x;
		}
	};
}

#endif
