#ifndef _PBOS_PBCORE_SYSCALL_PBCORE_H_
#define _PBOS_PBCORE_SYSCALL_PBCORE_H_

#include <pbos/generated/syscall/pbcore.h>

enum {
	PBCORE_OPEN_READ = 0x00000001,
	PBCORE_OPEN_WRITE = 0x00000002,
	PBCORE_OPEN_EXCLUSIVE = 0x00000020,
};

typedef uint32_t pbcore_open_flags_t;

#endif
