#ifndef _ARCH_I386_TSS_H_
#define _ARCH_I386_TSS_H_

#include <pbos/attribs.h>
#include <stdint.h>

typedef struct PB_PACKED _arch_tss_t {
    uint16_t link;
    uint16_t reserved0;
    uint32_t esp0;
    uint16_t ss0;
    uint16_t reserved1;
    uint32_t esp1;
    uint16_t ss1;
    uint16_t reserved2;
    uint32_t esp2;
    uint16_t ss2;
    uint16_t reserved3;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint16_t reserved4;
    uint32_t cs;
    uint16_t reserved5;
    uint32_t ss;
    uint16_t reserved6;
    uint32_t ds;
    uint16_t reserved7;
    uint32_t fs;
    uint16_t reserved8;
    uint32_t gs;
    uint16_t reserved9;
    uint32_t ldtr;
    uint16_t reserved10;
    uint16_t reserved11;
    uint16_t iobp;
    uint32_t ssp;
} arch_tss_t;

PB_FORCEINLINE static void arch_ltr(uint16_t tr) {
	__asm__ __volatile__("mov %0, %%ax" ::"m"(tr));
	__asm__ __volatile__("ltr %ax" );
}

#endif
