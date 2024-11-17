#ifndef _ARCH_I386_GATE_H_
#define _ARCH_I386_GATE_H_

#include <pbos/attribs.h>
#include <stdint.h>

typedef struct PB_PACKED _arch_gate_t {
	uint16_t off_low : 16;
	uint16_t selector : 16;
	uint8_t unused : 8;
	uint8_t attribs : 8;
	uint16_t off_high : 16;
} arch_gate_t;

#define GATEDESC(off, sel, attr)                    \
	((arch_gate_t){                                \
		.off_low = (((uint32_t)(off)) & 0x0000ffff), \
		.off_high = (((uint32_t)(off)) >> 16),       \
		.selector = (sel),                          \
		.attribs = (attr),                          \
	})

#define GATEDESC_OFFSET(l, h) ((void*)((l) | ((h) << 16)))

#define GATEDESC_ATTRIBS(p, dpl, s, type) ((type) | ((s) << 4) | ((dpl) << 5) | ((p) << 7))

#define GATE_TASK386 0x5
#define GATE_INT286 0x3
#define GATE_TRAP286 0x7
#define GATE_INT386 0xe
#define GATE_TRAP386 0xf

#endif
