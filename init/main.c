#include <pbos/attribs.h>
#include <pbos/common.h>

uint64_t user_syscall(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9);

const char INIT_PATH[] = "/initcar/pbinit";

PBOS_NORETURN void _start() {
	uint32_t fd;
	if(user_syscall(1, (uint64_t)INIT_PATH, sizeof(INIT_PATH) - 1, 0, 0, (uint64_t)&fd))
		;
	uint32_t proc_id;
	if(user_syscall(5, fd, fd, (uint64_t)"", 0, (uint64_t)&proc_id))
		;
	while (1) {
		asm volatile("nop");
	}

	user_syscall(114514, 1919810, 114, 514, 1919, 810);
}
