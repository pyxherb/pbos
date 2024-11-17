#include "int.h"

void arch_lidt(void *idt, uint16_t size)
{
	volatile struct PB_PACKED
	{
		uint16_t limit;
		void *base;
	} reg;

	reg.limit = (size << 3) - 1;
	reg.base = idt;
	__asm__ __volatile__("lidt %0" ::"m"(reg));
}
