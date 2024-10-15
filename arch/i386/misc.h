#ifndef _ARCH_I386_MISC_H_
#define _ARCH_I386_MISC_H_

#include <pbos/common.h>

#define arch_fence() __asm__ __volatile__("" ::: "memory")
#define arch_halt() __asm__ __volatile__("hlt");

#endif
