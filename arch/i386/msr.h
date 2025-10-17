#ifndef _ARCH_I386_MSR_H_
#define _ARCH_I386_MSR_H_

#include "cpuid.h"

#define arch_rdmsr(msr, l, h) asm volatile("rdmsr" : "=a"(*(l)), "=d"(*(h)) : "c"(msr))
#define arch_wrmsr(msr, l, h) asm volatile("wrmsr" :: "a"(l), "d"(h), "c"(msr))

PBOS_FORCEINLINE static bool arch_has_msr() {
	uint32_t a, b, c, d;
	arch_cpuid(1, &a, &b, &c, &d);
	return d & ARCH_CPUID_FEATURE_EDX_MSR;
}

#endif
