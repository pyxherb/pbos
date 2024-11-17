#ifndef _PBOS_ATTRIBS_H_
#define _PBOS_ATTRIBS_H_

// The function will never return
#if defined(__GNUC__) || defined(__clang__)
	#define PB_NORETURN __attribute__((__noreturn__))
#else
	#define PB_NORETURN
#endif

// Should not discard the return value
#ifdef __cplusplus
	#if __cplusplus >= 202002L
		#define PB_NODISCARD [[nodiscard("Should not discard the result")]]
	#elif __cplusplus >= 201703L
		#define PB_NODISCARD [[nodiscard]]
	#endif
#elif defined(__GNUC__) || defined(__clang__)
	#define PB_NODISCARD __attribute__((__warn_unused_result__))
#else
	#define PB_NODISCARD
#endif

// Always inline
#ifndef PB_FORCEINLINE
	#if defined(__GNUC__) || defined(__clang__)
		#define PB_FORCEINLINE __attribute__((__always_inline__)) inline
	#else
		#define PB_FORCEINLINE
	#endif
#endif

// No inline
#if defined(__GNUC__) || defined(__clang__)
	#define PB_NOINLINE __attribute__((PB_NOINLINE__))
#else
	#define PB_NOINLINE
#endif

// Indicates that the function behaves like several functions that takes formatted parameters.
#if defined(__GNUC__) || defined(__clang__)
	#define PB_FMTARG(type, idx_fmt, idx_args) __attribute__((__format__(type, idx_fmt, idx_args)))
#else
	#define PB_FMTARG(type, idx_fmt, idx_args)
#endif

// Use `stdcall` calling convention
#if defined(__GNUC__) || defined(__clang__)
	#define PB_STDCALL __attribute__((__stdcall__))
#else
	#define PB_STDCALL
#endif

// Use `cdecl` calling convention
#if defined(__GNUC__) || defined(__clang__)
	#define PB_CDECL __attribute__((__cdecl__))
#else
	#define PB_CDECL
#endif

// Use `fastcall` calling convention
#if defined(__GNUC__) || defined(__clang__)
	#define PB_FASTCALL __attribute__((__fastcall__))
#else
	#define PB_FASTCALL
#endif

// Use `naked` calling convention
#if defined(__GNUC__) || defined(__clang__)
	#define PB_NAKED __attribute__((__naked__))
#else
	#define PB_NAKED
#endif

// Packed
#if defined(__GNUC__) || defined(__clang__)
	#define PB_PACKED __attribute__((__packed__))
#else
	#define PB_PACKED
#endif

// Weak
#if defined(__GNUC__) || defined(__clang__)
	#define PB_WEAK __attribute__((__weak__))
#else
	#define PB_WEAK
#endif

// Weak reference
#if defined(__GNUC__) || defined(__clang__)
	#define PB_WEAKREF __attribute__((__weakref__))
#else
	#define PB_WEAKREF
#endif

// Put in a named section
#if defined(__GNUC__) || defined(__clang__)
	#define PB_IN_SECTION(s) __attribute__((__section__(s)))
#else
	#define PB_IN_SECTION(s)
#endif

// Shared library exported
#if defined(__GNUC__) || defined(__clang__)
	#ifdef _WIN32
		#define PB_EXPORTED __attribute__((dllexport))
	#else
		#define PB_EXPORTED __attribute__((__visibility__("default")))
	#endif
#else
	#define PB_EXPORTED
#endif

// Imported from shared library
#if defined(__GNUC__) || defined(__clang__)
	#ifdef _WIN32
		#define PB_IMPORTED __attribute__((dllimport))
	#else
		#define PB_IMPORTED
	#endif
#else
	#define PB_IMPORTED
#endif

// Deprecated
#if defined(__GNUC__) || defined(__clang__)
	#define PB_DEPRECATED __attribute__((__deprecated__))
#else
	#define PB_DEPRECATED
#endif

// Used
#if defined(__GNUC__) || defined(__clang__)
	#define PB_USED __attribute__((used))
#else
	#define PB_USED
#endif

#endif
