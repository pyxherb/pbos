#ifndef _PBOS_UTILS_H_
#define _PBOS_UTILS_H_

#include <stddef.h>

// Get length of an array in compile time.
#ifndef ARRAYLEN
	#define ARRAYLEN(a) (sizeof(a) / sizeof(*(a)))
#endif

// Check if 2 areas were overlapped.
#ifndef ISOVERLAPPED
	#define ISOVERLAPPED(p1, sz1, p2, sz2) (((p2) >= (p1) && (p2) < ((p1) + (sz1))) || ((p1) >= (p2) && (p1) <= ((p2) + (sz2))))
#endif

// Static assert
#ifndef static_assert
	#ifdef _Static_assert
		#define static_assert(c) _Static_assert(c);
	#else
		#define static_assert(c) typedef int static_assert[c];
	#endif
#endif

#ifndef typeof
	#ifdef __cplusplus
		#define typeof(x) decltype(x)
	#elif defined(__GNUC__) || defined(__clang__)
	#else
		#define typeof(x) static_assert(false);	 // Unsupported
	#endif
#endif

#define CONTAINER_OF(t, m, p) ((t *)(((char *)p) - offsetof(t, m)))

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define SIZE_FLOOR(x) ((x) & ~(sizeof(long) - 1))
#define SIZE_CEIL(x) ((x) & (sizeof(long) - 1) ? ((x) | (sizeof(long) - 1)) + 1 : (x))

#if defined(__GNUC__) || defined(__clang__)

	#define STDCALL_DECL(former, latter) former __attribute__((__stdcall__)) latter
	#define CDECL_DECL(former, latter) former __attribute__((__cdecl__)) latter
	#define FASTCALL_DECL(former, latter) former __attribute__((__fastcall__)) latter
	#define NAKED_DECL(former, latter) former __attribute__((__naked__)) latter

#elif defined(_MSC_VER)

	#define STDCALL_DECL(former, latter) former __stdcall latter
	#define CDECL_DECL(former, latter) former __cdecl latter
	#define FASTCALL_DECL(former, latter) former __fastcall latter
	#define NAKED_DECL(former, latter) former __naked latter

#else

	#define STDCALL_DECL(former, latter) former latter
	#define CDECL_DECL(former, latter) former latter
	#define FASTCALL_DECL(former, latter) former latter
	#define NAKED_DECL(former, latter) former latter

#endif

// Packed Scope
#ifdef _MSC_VER
	#define __packed_begin _Pragma("pack(push, 1)")
	#define __packed_end _Pragma("pack(pop)")
#else
	#define __packed_begin
	#define __packed_end
#endif

// Compiler Message
#ifdef _MSC_VER
	#define __message(msg) _Pragma("message(msg)")
#else
	#define __message(msg)
#endif

// Macro push & pop
#ifdef _MSC_VER
	#define __push_macro(name) _Pragma("push_macro("##name##")")
	#define __pop_macro(name) _Pragma("pop_macro("##name##")")
#else
	#define __push_macro
	#define __pop_macro
#endif

#endif
