#include <pbos/attribs.h>
#include <pbos/common.h>

uint32_t user_syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t edi);

const char INIT_PATH[] = "/initcar/pbinit";

PBOS_NORETURN void _start() {
	uint32_t fd;
	if(user_syscall(1, (uint32_t)INIT_PATH, sizeof(INIT_PATH) - 1, 0, 0, (uint32_t)&fd))
		;
	uint32_t proc_id;
	if(user_syscall(5, fd, fd, (uint32_t)"", 0, (uint32_t)&proc_id))
		;
	while (1) {
		asm volatile("nop");
	}
}
