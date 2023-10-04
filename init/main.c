#include <oicos/attribs.h>

void __noreturn _start() {
	while(1) {
		asm volatile("xchg %bx, %bx");
	}
}
