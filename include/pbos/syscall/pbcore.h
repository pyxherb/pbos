#ifndef _PBOS_PBCORE_SYSCALL_H_
#define _PBOS_PBCORE_SYSCALL_H_

#include <pbos/km/result.h>
#include <pbos/km/proc.h>

void *sysent_exit(int exitcode);
km_result_t sysent_open(const char *path, size_t path_len, uint32_t flags, uint32_t mode, ps_uhandle_t *uhandle_out);
void sysent_close(ps_uhandle_t uhandle, uint32_t flags);
void *sysent_read(ps_uhandle_t uhandle, void *buf, uint32_t size);
void *sysent_write(ps_uhandle_t uhandle, const void *buf, uint32_t size);
km_result_t sysent_exec_child(
	ps_uhandle_t file_handle,
	ps_uhandle_t cwd_handle,
	const char *args,
	size_t args_len,
	proc_id_t *proc_id_out);

#endif
