#ifndef _PBOS_KFXX_HASH_H_
#define _PBOS_KFXX_HASH_H_

#include "basedefs.hh"
#include <type_traits>

namespace kfxx {
	template <typename T>
	struct hash {
		static_assert(std::false_type::value, "Hasher not found");
	};

	template <>
	struct hash<signed char> {
		PBOS_FORCEINLINE size_t operator()(signed char x) const {
			return (size_t)x;
		}
	};

	template <>
	struct hash<unsigned char> {
		PBOS_FORCEINLINE size_t operator()(unsigned char x) const {
			return (size_t)x;
		}
	};

	template <>
	struct hash<char> {
		PBOS_FORCEINLINE size_t operator()(char x) const {
			return (size_t)x;
		}
	};

	template <>
	struct hash<short> {
		PBOS_FORCEINLINE size_t operator()(short x) const {
			return (size_t)x;
		}
	};

	template <>
	struct hash<unsigned short> {
		PBOS_FORCEINLINE size_t operator()(unsigned short x) const {
			return (size_t)x;
		}
	};

	template <>
	struct hash<int> {
		PBOS_FORCEINLINE size_t operator()(int x) const {
			return (size_t)x;
		}
	};

	template <>
	struct hash<unsigned int> {
		PBOS_FORCEINLINE size_t operator()(unsigned int x) const {
			return (size_t)x;
		}
	};

	template <>
	struct hash<long> {
		PBOS_FORCEINLINE size_t operator()(long x) const {
			return (size_t)x;
		}
	};

	template <>
	struct hash<unsigned long> {
		PBOS_FORCEINLINE size_t operator()(unsigned long x) const {
			return (size_t)x;
		}
	};

	template <>
	struct hash<long long> {
		PBOS_FORCEINLINE size_t operator()(long long x) const {
			return (size_t)x;
		}
	};

	template <>
	struct hash<unsigned long long> {
		PBOS_FORCEINLINE size_t operator()(unsigned long long x) const {
			return (size_t)x;
		}
	};
}

#endif
