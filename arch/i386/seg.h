#ifndef _ARCH_I386_SEG_H_
#define _ARCH_I386_SEG_H_

#include <oicos/attribs.h>
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
typedef struct __packed _arch_gdt_desc_t {
	uint16_t limit_low : 16;
	uint16_t base_low : 16;
	uint8_t base_mid : 8;
	uint8_t access_byte : 8;
	uint8_t limit_high : 4;
	uint8_t flags : 4;
	uint8_t base_high : 8;
} arch_gdt_desc_t;

///
/// @brief Initialize a GDT descriptor.
///
#define GDTDESC(base, limit, _access_byte, _flags)        \
	((arch_gdt_desc_t){ .base_low = ((base)&0x0000ffff),  \
		.base_mid = ((uint32_t)(base)&0x00ff0000 >> 16),  \
		.base_high = ((uint32_t)(base)&0xff000000 >> 24), \
		.limit_low = ((limit)&0x0ffff),                   \
		.limit_high = ((limit)&0xf0000 >> 16),            \
		.access_byte = (_access_byte),                    \
		.flags = (_flags) })

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
__always_inline static inline void arch_lgdt(void *gdt, uint16_t desc_num) {
	volatile struct __packed {
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
__always_inline static inline void arch_loadcs(uint16_t value) {
	bool loaded = false;

	struct __packed {
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

#endif
