#ifndef _HAL_I386_SYSCALL_H_
#define _HAL_I386_SYSCALL_H_

#include "irq.h"
#include "syscall/pbcore.h"

#define IRQ_SYSCALL 0xc0

uint32_t hn_syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t edi);

#endif
