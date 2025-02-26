#include <pbos/attribs.h>
#include <pbos/common.h>

uint32_t user_syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t edi);

const char INIT_PATH[] = "/initcar/pbinit";

PB_NORETURN void _start() {
	uint32_t fd;
	if(user_syscall(1, INIT_PATH, sizeof(INIT_PATH) - 1, 0, 0, &fd))
		asm volatile("xchg %bx, %bx");
	uint32_t proc_id;
	if(user_syscall(5, fd, fd, "", 0, (uint32_t)&proc_id))
		asm volatile("xchg %bx, %bx");;
	while (1) {
		asm volatile("nop");
	}
}
