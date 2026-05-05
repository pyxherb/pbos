#include "syscall.hh"
#include <pbos/km/logger.h>

PBOS_EXTERN_C_BEGIN

uint64_t hn_syscall_handler(uint64_t eax, uint64_t ebx, uint64_t ecx, uint64_t edx, uint64_t esi, uint64_t edi) {
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
			return sysent_exec_child((ps_ufd_t)ebx, (ps_ufd_t)ecx, (const char*)edx, (size_t)esi, (ps_proc_id_t *)edi);
		default:
			klog_printf("Test unpassed");
	}

	return 0;
}

PBOS_EXTERN_C_END
