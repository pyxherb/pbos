#ifndef _ARCH_I386_SEG_H_
#define _ARCH_I386_SEG_H_

#include <pbos/attribs.h>
#include <stdint.h>
#include "reg.h"

#define GDT_AB_P 0x80			  // Present
#define GDT_AB_DPL(n) ((n) << 5)  // DPL
#define GDT_AB_S 0x10			  // Storage Segment
#define GDT_AB_EX 0x8			  // Executable
#define GDT_AB_DC 0x4			  // Direction/Conforming
#define GDT_AB_RW 0x2			  // Read/Write
#define GDT_AB_A 0x1			  // Accessed

#define GDT_FL_L 0x2  // Long Mode
#define GDT_FL_DB 0x4
#define GDT_FL_GR 0x8  // Granularity

#define GDT_ACCESS_A(ab) ((ab)&0x01)		   // Accessed
#define GDT_ACCESS_RW(ab) (((ab)&0x02) >> 1)   // Read/Write
#define GDT_ACCESS_DC(ab) (((ab)&0x04) >> 2)   // Direction/Conforming
#define GDT_ACCESS_E(ab) (((ab)&0x08) >> 3)	   // Executable
#define GDT_ACCESS_S(ab) (((ab)&0x10) >> 4)	   // Descriptor Type
#define GDT_ACCESS_DPL(ab) (((ab)&0x20) >> 5)  // Privilege Level
#define GDT_ACCESS_P(ab) (((ab)&0x80) >> 7)	   // Present

#define GDT_FLAG_AVL(fl) ((fl)&0b0001)	// OS available
#define GDT_FLAG_L(fl) (((fl)&0b0010) >> 1)
#define GDT_FLAG_SZ(fl) (((fl)&0b0100) >> 2)
#define GDT_FLAG_GR(fl) (((fl)&0b1000) >> 3)

///
/// @brief The structure represents a GDT descriptor.
///
typedef uint64_t arch_gdt_desc_t;

///
/// @brief Initialize a GDT descriptor.
///
#define GDTDESC(base, limit, access_byte, flags)        \
	((arch_gdt_desc_t)\
	((((uint64_t)(limit)) & 0x0ffff)|\
	((((uint64_t)(base)) & 0x00ffffff) << 16)|\
	(((uint64_t)(access_byte)) << 40) |\
	((((uint64_t)(limit)) & 0xf0000) >> 16 << 48) |\
	(((uint64_t)(flags)) << 52) |\
	(((uint64_t)(base) & 0xff000000) >> 24 << 56)))

#define GDT_SYSTYPE_TSS16 0x1
#define GDT_SYSTYPE_LDT 0x2
#define GDT_SYSTYPE_TSS16BUSY 0x3
#define GDT_SYSTYPE_TSS32 0x9
#define GDT_SYSTYPE_TSS32BUSY 0xb

///
/// @brief Get base from a GDT descriptor.
///
#define GDTDESC_BASE(l, m, h) ((void *)(l | (m << 16) | (h << 24)))
///
/// @brief Get limit from a GDT descriptor.
///
#define GDTDESC_LIMIT(l, h) ((void *)(l | (h << 16)))

#define SELECTOR(rpl, ti, index) (rpl | ti << 2 | index << 3)

/// @brief Load GDT.
///
/// @param gdt Address of GDT.
/// @param desc_num Number of descriptors.
PB_FORCEINLINE static void arch_lgdt(void *gdt, uint16_t desc_num) {
	volatile struct PB_PACKED {
		uint16_t limit;
		void *base;
	} reg;

	reg.limit = (desc_num << 3) - 1;
	reg.base = gdt;
	__asm__ __volatile__("lgdt %0" ::"m"(reg));
}

/// @brief Load the selector to CS register.
///
/// @param value Selector to load.
PB_FORCEINLINE static void arch_loadcs(uint16_t value) {
	bool loaded = false;

	struct PB_PACKED {
		void *ptr;
		uint16_t value;
	} data;

	data.ptr = arch_reip();
	data.value = value;

	// Will be jumped to here.
	if (loaded)
		return;

	loaded = true;

	// Jump indirectly to load the selector.
	__asm__ __volatile__("ljmp *(%0)" ::"r"(&data));
}

#define arch_loadds(value) \
	__asm__ __volatile__("movw %0, %%ds" ::"r"((uint16_t)(value)))
#define arch_loadss(value) \
	__asm__ __volatile__("movw %0, %%ss" ::"r"((uint16_t)(value)))
#define arch_loades(value) \
	__asm__ __volatile__("movw %0, %%es" ::"r"((uint16_t)(value)))
#define arch_loadfs(value) \
	__asm__ __volatile__("movw %0, %%fs" ::"r"((uint16_t)(value)))
#define arch_loadgs(value) \
	__asm__ __volatile__("movw %0, %%gs" ::"r"((uint16_t)(value)))

PB_FORCEINLINE static uint16_t arch_storefs() {
	uint16_t value;
	__asm__ __volatile__("movw %fs, %ax");
	__asm__ __volatile__("movw %%ax, %0" :"=m"((value)));
	return value;
}

#endif
