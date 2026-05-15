#ifndef _PBOS_KFXX_HASH_H_
#define _PBOS_KFXX_HASH_H_

#include "basedefs.hh"
#include <type_traits>

namespace kfxx {
	template <typename T>
	struct Hash {
		static_assert(std::false_type::value, "Hasher not found");
	};

	template <>
	struct Hash<signed char> {
		PBOS_FORCEINLINE size_t operator()(signed char x) const {
			return (size_t)x;
		}
	};

	template <>
	struct Hash<unsigned char> {
		PBOS_FORCEINLINE size_t operator()(unsigned char x) const {
			return (size_t)x;
		}
	};

	template <>
	struct Hash<char> {
		PBOS_FORCEINLINE size_t operator()(char x) const {
			return (size_t)x;
		}
	};

	template <>
	struct Hash<short> {
		PBOS_FORCEINLINE size_t operator()(short x) const {
			return (size_t)x;
		}
	};

	template <>
	struct Hash<unsigned short> {
		PBOS_FORCEINLINE size_t operator()(unsigned short x) const {
			return (size_t)x;
		}
	};

	template <>
	struct Hash<int> {
		PBOS_FORCEINLINE size_t operator()(int x) const {
			return (size_t)x;
		}
	};

	template <>
	struct Hash<unsigned int> {
		PBOS_FORCEINLINE size_t operator()(unsigned int x) const {
			return (size_t)x;
		}
	};

	template <>
	struct Hash<long> {
		PBOS_FORCEINLINE size_t operator()(long x) const {
			return (size_t)x;
		}
	};

	template <>
	struct Hash<unsigned long> {
		PBOS_FORCEINLINE size_t operator()(unsigned long x) const {
			return (size_t)x;
		}
	};

	template <>
	struct Hash<long long> {
		PBOS_FORCEINLINE size_t operator()(long long x) const {
			return (size_t)x;
		}
	};

	template <>
	struct Hash<unsigned long long> {
		PBOS_FORCEINLINE size_t operator()(unsigned long long x) const {
			return (size_t)x;
		}
	};
}

#endif
