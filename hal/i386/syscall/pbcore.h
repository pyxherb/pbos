#ifndef _HAL_I386_SYSCALL_PBCORE_H_
#define _HAL_I386_SYSCALL_PBCORE_H_

#include <hal/i386/proc.h>
#include <hal/i386/syscall.h>

typedef enum _pbcore_sysent_id {
	SYSENT_PBCORE_EXIT = 0,
	SYSENT_PBCORE_OPEN,
	SYSENT_PBCORE_CLOSE,
	SYSENT_PBCORE_READ,
	SYSENT_PBCORE_WRITE,
	SYSENT_PBCORE_EXEC_CHILD
} pbcore_sysent_id;

void *sysent_exit(uint32_t exitcode);
int sysent_open(const char *path, uint32_t flags, uint32_t mode);
void sysent_close(int fd, uint32_t flags);
void *sysent_read(int fd, void *buf, uint32_t size);
void *sysent_write(int fd, const void *buf, uint32_t size);
km_result_t sysent_exec_child(const char *path, const char *arg, const char *cwd, proc_id_t *proc_id_out);

#endif
