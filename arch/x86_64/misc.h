#ifndef _ARCH_X86_64_MISC_H_
#define _ARCH_X86_64_MISC_H_

#include <pbos/common.h>

#define arch_fence() __asm__ __volatile__("" ::: "memory")
#define arch_hlt() __asm__ __volatile__("hlt");

#endif
