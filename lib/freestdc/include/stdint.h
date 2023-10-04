#ifndef _FREESTDC_STDINT_H_
#define _FREESTDC_STDINT_H_

#define INT8_MIN -128
#define INT16_MIN -32768
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN (-9223372036854775807 - 1)

#define INT8_MAX 127
#define INT16_MAX 32767
#define INT32_MAX 2147483647
#define INT64_MAX 9223372036854775807

#define UINT8_MAX 0xff
#define UINT16_MAX 0xffff
#define UINT32_MAX 0xffffffff
#define UINT64_MAX 0xffffffffffffffff

#define INTMAX_MIN INT64_MIN
#define INTMAX_MAX INT64_MAX
#define UINTMAX_MAX UINT64_MAX

#define INT_LEAST8_MIN INT8_MIN
#define INT_LEAST16_MIN INT16_MIN
#define INT_LEAST32_MIN INT32_MIN
#define INT_LEAST64_MIN INT64_MIN

#define INT_LEAST8_MAX INT8_MAX
#define INT_LEAST16_MAX INT16_MAX
#define INT_LEAST32_MAX INT32_MAX
#define INT_LEAST64_MAX INT64_MAX

#define UINT_LEAST8_MAX UINT8_MAX
#define UINT_LEAST16_MAX UINT16_MAX
#define UINT_LEAST32_MAX UINT32_MAX
#define UINT_LEAST64_MAX UINT64_MAX

#define INT_FAST8_MIN INT32_MIN
#define INT_FAST16_MIN INT32_MIN
#define INT_FAST32_MIN INT32_MIN
#define INT_FAST64_MIN INT64_MIN

#define INT_FAST8_MAX INT32_MAX
#define INT_FAST16_MAX INT32_MAX
#define INT_FAST32_MAX INT32_MAX
#define INT_FAST64_MAX INT64_MAX

#define UINT_FAST8_MAX UINT32_MAX
#define UINT_FAST16_MAX UINT32_MAX
#define UINT_FAST32_MAX UINT32_MAX
#define UINT_FAST64_MAX UINT64_MAX

#define INTPTR_MIN INT32_MIN
#define INTPTR_MAX INT32_MAX
#define UINTPTR_MAX UINT32_MAX

#define SIZE_MAX UINT32_MAX

#define PTRDIFF_MIN INT32_MIN
#define PTRDIFF_MAX INT32_MAX

#define SIG_ATOMIC_MIN INT32_MIN
#define SIG_ATOMIC_MAX INT32_MAX

#define WCHAR_MIN 0
#define WCHAR_MAX INT32_MAX

#define WINT_MIN INT16_MIN
#define WINT_MAX INT16_MAX

#define INTMAX_C(n) (n##i32)
#define UNTMAX_C(n) (n##ui32)

#define INT8_C(n) (n##i8)
#define INT16_C(n) (n##i16)
#define INT32_C(n) (n##i32)
#define INT64_C(n) (n##i64)

#define UINT8_C(n) (n##ui8)
#define UINT16_C(n) (n##ui16)
#define UINT32_C(n) (n##ui32)
#define UINT64_C(n) (n##ui64)

typedef long intmax_t;
typedef unsigned long uintmax_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_least_t;
typedef signed short int16_least_t;
typedef signed int int32_least_t;
typedef signed long long int64_least_t;

typedef unsigned char uint8_least_t;
typedef unsigned short uint16_least_t;
typedef unsigned int uint32_least_t;
typedef unsigned long long uint64_least_t;

typedef signed int int8_fast_t;
typedef signed int int16_fast_t;
typedef signed int int32_fast_t;
typedef signed long long int64_fast_t;

typedef unsigned int uint8_fast_t;
typedef unsigned int uint16_fast_t;
typedef unsigned int uint32_fast_t;
typedef unsigned long long uint64_fast_t;

#endif
