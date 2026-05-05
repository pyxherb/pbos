#ifndef _HAL_X86_64_SYSCALL_H_
#define _HAL_X86_64_SYSCALL_H_

#include <hal/x86_64/irq.hh>
#include <pbos/syscall/pbcore.h>

#define IRQ_SYSCALL 0xc0

PBOS_EXTERN_C_BEGIN

PBOS_NORETURN void isr_syscall();

uint64_t hn_syscall_handler(uint64_t eax, uint64_t ebx, uint64_t ecx, uint64_t edx, uint64_t esi, uint64_t edi);

PBOS_EXTERN_C_END

#endif
