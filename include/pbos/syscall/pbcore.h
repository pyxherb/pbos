#ifndef _PBOS_PBCORE_SYSCALL_H_
#define _PBOS_PBCORE_SYSCALL_H_

#include <pbos/km/result.h>
#include <pbos/km/proc.h>

void *sysent_exit(int exitcode);
km_result_t sysent_open(const char *path, size_t path_len, uint32_t flags, uint32_t mode, ps_ufd_t *ufd_out);
km_result_t sysent_close(ps_ufd_t ufd, uint32_t flags);
void *sysent_read(ps_ufd_t ufd, void *buf, uint32_t size);
void *sysent_write(ps_ufd_t ufd, const void *buf, uint32_t size);
km_result_t sysent_exec_child(
	ps_ufd_t file_ufd,
	ps_ufd_t cwd_ufd,
	const char *args,
	size_t args_len,
	proc_id_t *proc_id_out);

#endif
