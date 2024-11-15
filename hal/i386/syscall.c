#include "syscall.h"
#include <pbos/km/logger.h>

void hn_syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t edi) {
    switch(eax) {
        case 0:
            kprintf("syscall test\n");
            break;
        default:
            kputs("Test unpassed");
    }
}