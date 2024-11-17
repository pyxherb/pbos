#include <pbos/attribs.h>

char test[512];

PB_NORETURN void _start() {
	while (1) {
		for (int i = 0; i < 512; ++i)
			test[i] = i;
		asm volatile("movl $0, %eax");
		asm volatile("int $0x01");
		asm volatile("xchg %bx, %bx");
	}
}
