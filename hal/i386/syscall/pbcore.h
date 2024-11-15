#ifndef _HAL_I386_SYSCALL_PBOS_H_
#define _HAL_I386_SYSCALL_PBOS_H_

#include <hal/i386/syscall.h>

void* sysent_exit(uint32_t exitcode);
void* sysent_read(int fd, void* buf, uint32_t size);
void* sysent_write(int fd, const void* buf, uint32_t size);
int sysent_open(const char* path, uint32_t flags, uint32_t mode);
void sysent_close(int fd, uint32_t flags);

#endif
