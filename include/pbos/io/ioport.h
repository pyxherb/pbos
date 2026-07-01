#ifndef _PBOS_IO_IOPORT_H_
#define _PBOS_IO_IOPORT_H_

#include <pbos/kf/basedefs.h>

PBOS_API bool io_is_ioport_supported();

PBOS_API void io_write_ioport8(uint16_t port, uint8_t data);
PBOS_API uint8_t io_read_ioport8(uint16_t port);
PBOS_API void io_write_ioport16(uint16_t port, uint16_t data);
PBOS_API uint16_t io_read_ioport16(uint16_t port);
PBOS_API void io_write_ioport32(uint16_t port, uint32_t data);
PBOS_API uint32_t io_read_ioport32(uint16_t port);

#endif
