#ifndef _PBOS_KD_GDB_H_
#define _PBOS_KD_GDB_H_

#include <pbos/km/result.h>

PBOS_EXTERN_C_BEGIN

typedef struct _kd_gdb_stub_backend_t {
	km_result_t (*init)();
	void (*deinit)();
	void (*send)(const char *src, size_t size);
	char (*recv_char)();
	void (*flush)();
} kd_gdb_stub_backend_t;

PBOS_API int kd_gdb_hex_to_digit(char digit);
PBOS_API char kd_digit_to_gdb_hex(int num);
PBOS_API uint8_t kd_compute_gdb_checksum(const char *data, size_t len);

PBOS_API km_result_t kd_set_gdb_stub_backend(kd_gdb_stub_backend_t *backend);

PBOS_EXTERN_C_END

#endif
