#ifndef _HAL_X86_64_SYSCALL_H_
#define _HAL_X86_64_SYSCALL_H_

#include <hal/x86_64/irq.hh>
#include <pbos/syscall/pbcore.h>

#define IRQ_SYSCALL 0xc0

PBOS_EXTERN_C_BEGIN

PBOS_NORETURN void isr_syscall();

PBOS_EXTERN_C_END

#endif
