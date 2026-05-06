#ifndef _ARCH_X86_64_TSS_H_
#define _ARCH_X86_64_TSS_H_

#include <pbos/attribs.h>
#include <stdint.h>

typedef struct PBOS_PACKED _arch_tss_t {
	uint32_t reserved0;
	uint64_t rsp0;
	uint64_t rsp1;
	uint64_t rsp2;
	uint64_t reserved1;
	uint64_t ist1;
	uint64_t ist2;
	uint64_t ist3;
	uint64_t ist4;
	uint64_t ist5;
	uint64_t ist6;
	uint64_t ist7;
	uint64_t reserved2;
	uint16_t reserved3;
	uint16_t iopb;
} arch_tss_t;

PBOS_FORCEINLINE static void arch_ltr(uint16_t tr) {
	__asm__ __volatile__("mov %0, %%ax" ::"m"(tr));
	__asm__ __volatile__("ltr %ax" );
}

#endif
