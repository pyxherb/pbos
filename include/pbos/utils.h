#ifndef _PBOS_UTILS_H_
#define _PBOS_UTILS_H_

#include <stddef.h>

// Get length of an array in compile time.
#define PB_ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)))

// Check if 2 areas are overlapped.
#define PB_ISOVERLAPPED(p1, sz1, p2, sz2) (((p2) >= (p1) && (p2) < ((p1) + (sz1))) || ((p1) >= (p2) && (p1) < ((p2) + (sz2))))

// Static assert
#ifndef PB_STATIC_ASSERT
	#ifdef _Static_assert
		#define PB_STATIC_ASSERT(c) _Static_assert(c);
	#else
		#define PB_STATIC_ASSERT(c) typedef int pb_static_assert_##__LINE__##_t[c];
	#endif
#endif

#define PB_CONTAINER_OF(t, m, p) ((t *)(((char *)p) - offsetof(t, m)))

#define PB_MIN(x, y) ((x) < (y) ? (x) : (y))
#define PB_MAX(x, y) ((x) > (y) ? (x) : (y))

#define PB_SIZEFLOOR(x) ((x) & ~(sizeof(long) - 1))
#define PB_SIZECEIL(x) ((x) & (sizeof(long) - 1) ? ((x) | (sizeof(long) - 1)) + 1 : (x))

#if defined(__GNUC__) || defined(__clang__)

	#define STDCALL_DECL(former, latter) former __attribute__((__stdcall__)) latter
	#define CDECL_DECL(former, latter) former __attribute__((__cdecl__)) latter
	#define FASTCALL_DECL(former, latter) former __attribute__((__fastcall__)) latter
	#define NAKED_DECL(former, latter) former __attribute__((__naked__)) latter

#elif defined(_MSC_VER)

	#define STDCALL_DECL(former, latter) former PB_STDCALL latter
	#define CDECL_DECL(former, latter) former PB_CDECL latter
	#define FASTCALL_DECL(former, latter) former PB_FASTCALL latter
	#define NAKED_DECL(former, latter) former PB_NAKED latter

#else

	#define STDCALL_DECL(former, latter) former latter
	#define CDECL_DECL(former, latter) former latter
	#define FASTCALL_DECL(former, latter) former latter
	#define NAKED_DECL(former, latter) former latter

#endif

// Packed Scope
#ifdef _MSC_VER
	#define PB_PACKED_BEGIN _Pragma("pack(push, 1)")
	#define PB_PACKED_END _Pragma("pack(pop)")
#else
	#define PB_PACKED_BEGIN
	#define PB_PACKED_END
#endif

// Compiler Message
#ifdef _MSC_VER
	#define PB_COMPILER_MESSAGE(msg) _Pragma("message(msg)")
#else
	#define PB_COMPILER_MESSAGE(msg)
#endif

// Macro push & pop
#ifdef _MSC_VER
	#define PB_PUSH_MACRO(name) _Pragma("push_macro("##name##")")
	#define PB_POP_MACRO(name) _Pragma("pop_macro("##name##")")
#else
	#define PB_PUSH_MACRO
	#define PB_POP_MACRO
#endif

#endif
