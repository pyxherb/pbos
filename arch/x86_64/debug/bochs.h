#ifndef _ARCH_X86_64_DEBUG_BOCHS_H_
#define _ARCH_X86_64_DEBUG_BOCHS_H_

#include <pbos/common.h>

#define arch_bochs_break() __asm__ __volatile__("xchgw %bx,%bx");

#endif
