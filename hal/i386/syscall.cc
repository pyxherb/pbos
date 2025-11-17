#include "syscall.h"
#include <pbos/km/logger.h>

PBOS_EXTERN_C_BEGIN

uint32_t hn_syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t edi) {
	switch (eax) {
		case SYSENT_PBCORE_EXIT:
			klog_printf("exit test\n");
			break;
		case SYSENT_PBCORE_OPEN:
			klog_printf("Open test");
			return sysent_open((const char *)ebx, (size_t)ecx, edx, esi, (ps_ufd_t *)edi);
		case SYSENT_PBCORE_CLOSE:
			klog_printf("Close test");
			return sysent_close((ps_ufd_t)ebx, ecx);
		case SYSENT_PBCORE_READ:
			klog_printf("Read test");
			break;
		case SYSENT_PBCORE_WRITE:
			klog_printf("Write test");
			break;
		case SYSENT_PBCORE_EXEC_CHILD:
			klog_printf("Exec child test");
			return sysent_exec_child((ps_ufd_t)ebx, (ps_ufd_t)ecx, (const char*)edx, (size_t)esi, (proc_id_t *)edi);
		default:
			klog_printf("Test unpassed, eax = %u", eax);
	}

	return 0;
}

PBOS_EXTERN_C_END
