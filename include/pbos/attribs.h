#ifndef _PBOS_ATTRIBS_H_
#define _PBOS_ATTRIBS_H_

// The function will never return
#if defined(__GNUC__) || defined(__clang__)
	#define __noreturn __attribute__((__noreturn__))
#else
	#define __noreturn
#endif

// Should not discard the return value
#ifdef __cplusplus
	#if __cplusplus >= 202002L
		#define __nodiscard [[nodiscard("Should not discard the result")]]
	#elif __cplusplus >= 201703L
		#define __nodiscard [[nodiscard]]
	#endif
#elif defined(__GNUC__) || defined(__clang__)
	#define __nodiscard __attribute__((__warn_unused_result__))
#else
	#define __nodiscard
#endif

// Always inline
#ifndef FORCEINLINE
	#if defined(__GNUC__) || defined(__clang__)
		#define FORCEINLINE __attribute__((__always_inline__)) inline
	#else
		#define FORCEINLINE
	#endif
#endif

// No inline
#if defined(__GNUC__) || defined(__clang__)
	#define __noinline __attribute__((__noinline__))
#else
	#define __noinline
#endif

// Indicates that the function acts like several functions that have formatted parameters.
#if defined(__GNUC__) || defined(__clang__)
	#define __format(type, idx_fmt, idx_args) __attribute__((__format__(type, idx_fmt, idx_args)))
#else
	#define __format(type, idx_fmt, idx_args)
#endif

// Returns non-null
#ifndef __returns_nonnull
	#if defined(__GNUC__) || defined(__clang__)
		#define __returns_nonnull __attribute__((__noreturns_nonnull__))
	#else
		#define __returns_nonnull
	#endif
#endif

// Use `stdcall` calling convention
#if defined(__GNUC__) || defined(__clang__)
	#define __stdcall __attribute__((__stdcall__))
#else
	#define __stdcall
#endif

// Use `cdecl` calling convention
#if defined(__GNUC__) || defined(__clang__)
	#define __cdecl __attribute__((__cdecl__))
#else
	#define __cdecl
#endif

// Use `fastcall` calling convention
#if defined(__GNUC__) || defined(__clang__)
	#define __fastcall __attribute__((__fastcall__))
#else
	#define __fastcall
#endif

// Use `naked` calling convention
#if defined(__GNUC__) || defined(__clang__)
	#define __naked __attribute__((__naked__))
#else
	#define __naked
#endif

// Packed
#if defined(__GNUC__) || defined(__clang__)
	#define __packed __attribute__((__packed__))
#else
	#define __packed
#endif

// Weak
#if defined(__GNUC__) || defined(__clang__)
	#define __weak __attribute__((__weak__))
#else
	#define __weak
#endif

// Weak reference
#if defined(__GNUC__) || defined(__clang__)
	#define __weakref __attribute__((__weakref__))
#else
	#define __weakref
#endif

// Put in a named section
#if defined(__GNUC__) || defined(__clang__)
	#define __in_section(s) __attribute__((__section__(s)))
#else
	#define __in_section(s)
#endif

// Shared library exported
#if defined(__GNUC__) || defined(__clang__)
	#ifdef _WIN32
		#define __exported __attribute__((dllexport))
	#else
		#define __exported __attribute__((__visibility__("default")))
	#endif
#else
	#define __exported
#endif

// Imported from shared library
#if defined(__GNUC__) || defined(__clang__)
	#ifdef _WIN32
		#define __imported __attribute__((dllimport))
	#else
		#define __imported
	#endif
#else
	#define __imported
#endif

// Deprecated
#if defined(__GNUC__) || defined(__clang__)
	#define __deprecated __attribute__((__deprecated__))
#else
	#define __deprecated
#endif

// `malloc'-like function
#if defined(__GNUC__) || defined(__clang__)
	#define __malloc __attribute__((__malloc__))
#else
	#define __malloc
#endif

#if defined(__GNUC__) || defined(__clang__)
	#define __allocsize(idx_size_param) __attribute__((__alloc_size__(idx_size_param)))
#else
	#define __allocsize(idx_size_param)
#endif

#if defined(__GNUC__) || defined(__clang__)
	#define __callocsize(idx_size_param, idx_blknum_param) __attribute__((__alloc_size__(idx_size_param, ##idx_blknum_param)))
#else
	#define __callocsize(idx_size_param, idx_blknum_param)
#endif

// Used
#if defined(__GNUC__) || defined(__clang__)
	#define __used __attribute__((used))
#else
	#define __used
#endif

#endif
