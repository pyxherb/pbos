#ifndef _ARCH_X86_64_MSR_H_
#define _ARCH_X86_64_MSR_H_

#include "cpuid.h"

#define ARCH_MSR_APIC_BASE 0x1b

PBOS_FORCEINLINE void arch_rdmsr(uint32_t msr, uint32_t *l, uint32_t *h) {
	asm volatile("rdmsr" : "=a"(*l), "=d"(*h) : "c"(msr));
}

PBOS_FORCEINLINE void arch_wrmsr(uint32_t msr, uint32_t l, uint32_t h) {
	asm volatile("wrmsr" : : "a"(l), "d"(h), "c"(msr));
}

#endif
