#ifndef _PBOS_KFXX_BITOPS_HH_
#define _PBOS_KFXX_BITOPS_HH_

#include "basedefs.hh"

namespace kfxx {
	PBOS_FORCEINLINE uint8_t count_leading_zero(uint8_t value) {
#if (defined(_M_IX86) || defined(_M_X64) || __i386__ || __x86_64__)
	#if defined(_MSC_VER)
		if (!value)
			return 8;
		return (uint8_t)_lzcnt_u32((uint32_t)value) - (sizeof(uint32_t) - sizeof(uint8_t)) * 8;
	#elif defined(__GNUC__) || defined(__clang__)
		if (!value)
			return 8;
		return __builtin_clz((uint32_t)value);
	#endif
#endif
		if (!value)
			return 8;
		uint8_t cnt = 0;
		value = ~value;
		while (value & 0x80) {
			value <<= 1;
			++cnt;
		}
		return cnt;
	}

	PBOS_FORCEINLINE uint8_t count_leading_zero(uint16_t value) {
#if (defined(_M_IX86) || defined(_M_X64) || __i386__ || __x86_64__)
	#ifdef _MSC_VER
		if (!value)
			return 16;
		return (uint8_t)_lzcnt_u32((uint32_t)value) - (sizeof(uint32_t) - sizeof(uint16_t)) * 8;
	#elif defined(__GNUC__) || defined(__clang__)
		if (!value)
			return 16;
		return __builtin_clz((uint32_t)value);
	#endif
#endif
		if (!value)
			return 16;
		uint8_t cnt = 0;
		value = ~value;
		while (value & 0x8000) {
			value <<= 1;
			++cnt;
		}
		return cnt;
	}

	PBOS_FORCEINLINE uint8_t count_leading_zero(uint32_t value) {
#if (defined(_M_IX86) || defined(_M_X64) || __i386__ || __x86_64__)
	#ifdef _MSC_VER
		if (!value)
			return 32;
		return (uint8_t)_lzcnt_u32((uint32_t)value);
	#elif defined(__GNUC__) || defined(__clang__)
		if (!value)
			return 32;
		return __builtin_clz((uint32_t)value);
	#endif
#endif
		if (!value)
			return 32;
		uint8_t cnt = 0;
		value = ~value;
		while (value & 0x80000000U) {
			value <<= 1;
			++cnt;
		}
		return cnt;
	}

	PBOS_FORCEINLINE uint8_t count_leading_zero(uint64_t value) {
#if (defined(_M_IX86) || defined(_M_X64) || __i386__ || __x86_64__)
	#ifdef _MSC_VER
		if (!value)
			return 64;
		return (uint8_t)_lzcnt_u64((uint32_t)value);
	#elif defined(__GNUC__) || defined(__clang__)
		if (!value)
			return 64;
		return __builtin_clz((uint32_t)value);
	#endif
#endif
		if (!value)
			return 64;
		uint8_t cnt = 0;
		value = ~value;
		while (value & 0x8000000000000000ULL) {
			value <<= 1;
			++cnt;
		}
		return cnt;
	}

	PBOS_FORCEINLINE uint8_t count_leading_zero(int8_t value) {
		return count_leading_zero((uint8_t)value);
	}

	PBOS_FORCEINLINE uint8_t count_leading_zero(int16_t value) {
		return count_leading_zero((uint16_t)value);
	}

	PBOS_FORCEINLINE uint8_t count_leading_zero(int32_t value) {
		return count_leading_zero((uint32_t)value);
	}

	PBOS_FORCEINLINE uint8_t count_leading_zero(int64_t value) {
		return count_leading_zero((uint64_t)value);
	}

	PBOS_FORCEINLINE uint8_t r_rot(uint8_t value, uint_fast8_t shift) {
#if (defined(_M_IX86) || defined(_M_X64) || __i386__ || __x86_64__)
	#ifdef _MSC_VER
		return _rotr8(value, shift);
	#endif
#endif
		return shift ? ((value >> shift) | (value << (8 - shift))) : value;
	}

	PBOS_FORCEINLINE uint16_t r_rot(uint16_t value, uint_fast8_t shift) {
#if (defined(_M_IX86) || defined(_M_X64) || __i386__ || __x86_64__)
	#ifdef _MSC_VER
		return _rotr16(value, shift);
	#endif
#endif
		return shift ? ((value >> shift) | (value << (16 - shift))) : value;
	}

	PBOS_FORCEINLINE uint32_t r_rot(uint32_t value, uint_fast8_t shift) {
#if (defined(_M_IX86) || defined(_M_X64) || __i386__ || __x86_64__)
	#ifdef _MSC_VER
		return _rotr(value, shift);
	#endif
#endif
		return shift ? ((value >> shift) | (value << (32 - shift))) : value;
	}

	PBOS_FORCEINLINE uint64_t r_rot(uint64_t value, uint_fast8_t shift) {
#if (defined(_M_IX86) || defined(_M_X64) || __i386__ || __x86_64__)
	#ifdef _MSC_VER
		return _rotr64(value, shift);
	#endif
#endif
		return shift ? ((value >> shift) | (value << (64 - shift))) : value;
	}

	PBOS_FORCEINLINE int8_t r_rot(int8_t value, uint_fast8_t shift) {
		return r_rot((uint8_t)value, shift);
	}

	PBOS_FORCEINLINE int16_t r_rot(int16_t value, uint_fast8_t shift) {
		return r_rot((uint16_t)value, shift);
	}

	PBOS_FORCEINLINE int32_t r_rot(int32_t value, uint_fast8_t shift) {
		return r_rot((uint32_t)value, shift);
	}

	PBOS_FORCEINLINE int64_t r_rot(int64_t value, uint_fast8_t shift) {
		return r_rot((uint64_t)value, shift);
	}

	PBOS_FORCEINLINE uint8_t l_rot(uint8_t value, uint_fast8_t shift) {
#if (defined(_M_IX86) || defined(_M_X64) || __i386__ || __x86_64__)
	#ifdef _MSC_VER
		return _rotl8(value, shift);
	#endif
#endif
		return shift ? ((value << shift) | (value >> (8 - shift))) : value;
	}

	PBOS_FORCEINLINE uint16_t l_rot(uint16_t value, uint_fast8_t shift) {
#if (defined(_M_IX86) || defined(_M_X64) || __i386__ || __x86_64__)
	#ifdef _MSC_VER
		return _rotl16(value, shift);
	#endif
#endif
		return shift ? ((value << shift) | (value >> (16 - shift))) : value;
	}

	PBOS_FORCEINLINE uint32_t l_rot(uint32_t value, uint_fast8_t shift) {
#if (defined(_M_IX86) || defined(_M_X64) || __i386__ || __x86_64__)
	#ifdef _MSC_VER
		return _rotl(value, shift);
	#endif
#endif
		return shift ? ((value << shift) | (value >> (32 - shift))) : value;
	}

	PBOS_FORCEINLINE uint64_t l_rot(uint64_t value, uint_fast8_t shift) {
#if (defined(_M_IX86) || defined(_M_X64) || __i386__ || __x86_64__)
	#ifdef _MSC_VER
		return _rotl64(value, shift);
	#endif
#endif
		return shift ? ((value << shift) | (value >> (32 - shift))) : value;
	}

	PBOS_FORCEINLINE int8_t l_rot(int8_t value, uint_fast8_t shift) {
		return l_rot((uint8_t)value, shift);
	}

	PBOS_FORCEINLINE int16_t l_rot(int16_t value, uint_fast8_t shift) {
		return l_rot((uint16_t)value, shift);
	}

	PBOS_FORCEINLINE int32_t l_rot(int32_t value, uint_fast8_t shift) {
		return l_rot((uint32_t)value, shift);
	}

	PBOS_FORCEINLINE int64_t l_rot(int64_t value, uint_fast8_t shift) {
		return l_rot((uint64_t)value, shift);
	}
}

#endif