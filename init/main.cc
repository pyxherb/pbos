#include <pbos/attribs.h>
#include <pbos/common.h>

PBOS_EXTERN_C_BEGIN

typedef void (*hn_ctor_t)();
typedef void (*hn_dtor_t)();

alignas(hn_ctor_t) hn_ctor_t KI_CTORS_BEGIN[0] = {};
alignas(hn_ctor_t) hn_ctor_t KI_CTORS_END[0] = {};

alignas(hn_dtor_t) hn_dtor_t KI_DTORS_BEGIN[0] = {};
alignas(hn_dtor_t) hn_dtor_t KI_DTORS_END[0] = {};

extern "C" void __cxa_pure_virtual() {
}

void hal_call_ctors() {
	const size_t n_ctors = KI_CTORS_END - KI_CTORS_BEGIN;
	for (size_t i = 0; i < n_ctors; ++i) {
		KI_CTORS_BEGIN[i]();
	}
}

uint64_t user_syscall(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9);

const char INIT_PATH[] = "/initcar/pbinit";

PBOS_NORETURN void _start() {
	hal_call_ctors();
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

PBOS_EXTERN_C_END
