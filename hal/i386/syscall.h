#ifndef _HAL_I386_SYSCALL_H_
#define _HAL_I386_SYSCALL_H_

#include "irq.h"

#define IRQ_SYSCALL 0x80

typedef void* (*syscall_proc_t)(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t edi);

extern syscall_proc_t* _irq_syscall_subsets[];

#endif
