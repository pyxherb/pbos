#ifndef _ARCH_I386_DEBUG_BOCHS_H_
#define _ARCH_I386_DEBUG_BOCHS_H_

#include <oicos/common.h>

#define arch_bochs_break() __asm__ __volatile__("xchgw %bx,%bx");

#endif
