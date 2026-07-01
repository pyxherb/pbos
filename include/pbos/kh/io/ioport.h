#ifndef _PBOS_KH_IO_IOPORT_H_
#define _PBOS_KH_IO_IOPORT_H_

#include <pbos/io/ioport.h>

PBOS_API bool kh_is_ioport_supported();

PBOS_API void kh_write_ioport8(uint16_t port, uint8_t data);
PBOS_API uint8_t kh_read_ioport8(uint16_t port);
PBOS_API void kh_write_ioport16(uint16_t port, uint16_t data);
PBOS_API uint16_t kh_read_ioport16(uint16_t port);
PBOS_API void kh_write_ioport32(uint16_t port, uint32_t data);
PBOS_API uint32_t kh_read_ioport32(uint16_t port);

#endif
