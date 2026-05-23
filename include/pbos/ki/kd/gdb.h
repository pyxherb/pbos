#ifndef _PBOS_KI_KD_GDB_H_
#define _PBOS_KI_KD_GDB_H_

#include <pbos/kd/gdb.h>
#include <pbos/ps/mutex.hh>

PBOS_EXTERN_C_BEGIN

PBOS_PRIVATE extern ps::mutex_t ki_gdb_read_pmem_mapping_mutex;
PBOS_PRIVATE extern void *ki_gdb_read_pmem_mapping_vaddr;
PBOS_PRIVATE extern kd_gdb_stub_backend_t ki_kd_current_gdb_stub_backend;

PBOS_PRIVATE void ki_kd_send_packet(const char *data, size_t len);
PBOS_PRIVATE int ki_kd_recv_packet(char *buffer, size_t len);
PBOS_PRIVATE void ki_kd_send_ack();
PBOS_PRIVATE void ki_kd_send_nak();
PBOS_NORETURN PBOS_PRIVATE void ki_enter_gdb_stub(void *registers);

PBOS_EXTERN_C_END

#endif
