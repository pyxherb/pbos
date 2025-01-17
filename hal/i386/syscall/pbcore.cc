#include "pbcore.h"

void *sysent_exit(uint32_t exitcode) {

}

int sysent_open(const char *path, uint32_t flags, uint32_t mode) {

}

void sysent_close(int fd, uint32_t flags) {

}

void *sysent_read(int fd, void *buf, uint32_t size) {

}

void *sysent_write(int fd, const void *buf, uint32_t size) {

}

km_result_t sysent_exec_child(const char *path, const char *arg, const char *cwd, proc_id_t *proc_id_out) {

}
