#include "cpuid.h"

void arch_cpuid(uint32_t func, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
	__asm__ __volatile__("movl %0, %%eax" ::"m"(func));

	__asm__ __volatile__("cpuid");

	if (eax) {
		__asm__ __volatile__("pushl %eax");
		__asm__ __volatile__("popl %0"
							 : "=m"(*eax));
	}
	if (ebx) {
		__asm__ __volatile__("pushl %ebx");
		__asm__ __volatile__("popl %0"
							 : "=m"(*ebx));
	}
	if (ecx) {
		__asm__ __volatile__("pushl %ecx");
		__asm__ __volatile__("popl %0"
							 : "=m"(*ecx));
	}
	if (edx) {
		__asm__ __volatile__("pushl %edx");
		__asm__ __volatile__("popl %0"
							 : "=m"(*edx));
	}
}
