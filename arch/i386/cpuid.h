#ifndef _ARCH_I386_CPUID_H_
#define _ARCH_I386_CPUID_H_

#include <pbos/common.h>

#define CPUID_FUNC_VENDOR 0x00000000
#define CPUID_FUNC_CPU_NAME1 0x80000002
#define CPUID_FUNC_CPU_NAME2 0x80000003
#define CPUID_FUNC_CPU_NAME3 0x80000004

PB_FORCEINLINE static bool arch_iscpuidcap() {
	bool result = 0;

	__asm__ __volatile__("pushfl");
	__asm__ __volatile__("popl %eax");
	__asm__ __volatile__("andl 0x00200000,%eax");
	__asm__ __volatile__("cmpl 0x00200000,%eax");
	__asm__ __volatile__("jne 0f");
	__asm__ __volatile__("notb %0"
						 : "=m"(result));
	__asm__ __volatile__("0:");

	return result;
}

void arch_cpuid(uint32_t func, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx);

#endif
