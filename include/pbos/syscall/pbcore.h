#ifndef _PBOS_PBCORE_SYSCALL_PBCORE_H_
#define _PBOS_PBCORE_SYSCALL_PBCORE_H_

#include <pbos/km/result.h>
#include <pbos/km/proc.h>
#include <pbos/generated/syscall/pbcore.h>

PBOS_EXTERN_C_BEGIN

km_result_t sysent_exit(int exitcode);
km_result_t sysent_open(const char *path, size_t path_len, uint32_t flags, uint32_t mode, ps_ufd_t *ufd_out);
km_result_t sysent_close(ps_ufd_t ufd, uint32_t flags);
km_result_t sysent_read(ps_ufd_t ufd, void *buf, uint32_t size);
km_result_t sysent_write(ps_ufd_t ufd, const void *buf, uint32_t size);
km_result_t sysent_exec_child(
	ps_ufd_t file_ufd,
	ps_ufd_t cwd_ufd,
	const char *args,
	size_t args_len,
	ps_proc_id_t *proc_id_out);
km_result_t sysent_fork();

PBOS_EXTERN_C_END

#endif
