#ifndef _HAL_I386_SYSCALL_PBCORE_H_
#define _HAL_I386_SYSCALL_PBCORE_H_

#include <hal/i386/proc.hh>
#include <pbos/syscall/pbcore.h>

typedef enum _pbcore_sysent_id {
	SYSENT_PBCORE_EXIT = 0,
	SYSENT_PBCORE_OPEN,
	SYSENT_PBCORE_CLOSE,
	SYSENT_PBCORE_READ,
	SYSENT_PBCORE_WRITE,
	SYSENT_PBCORE_EXEC_CHILD
} pbcore_sysent_id;

#endif
