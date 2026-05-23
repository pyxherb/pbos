#ifndef _PBOS_KH_KD_GDB_H_
#define _PBOS_KH_KD_GDB_H_

#include <pbos/ki/kd/gdb.h>

PBOS_EXTERN_C_BEGIN

extern const char *kh_gdb_target_xml;

PBOS_PRIVATE void kh_handle_gdb_packet(const char *packet, size_t len, void *registers);

PBOS_EXTERN_C_END

#endif
