#include <pbos/attribs.h>

char test[512];

void __noreturn _start() {
	while (1) {
		for (int i = 0; i < 512; ++i)
			test[i] = i;
		asm volatile("xchg %bx, %bx");
		*((char*)0x80000000) = 0xcc;
	}
}
