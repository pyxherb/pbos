#include <pbos/kh/io/ioport.h>

PBOS_API bool io_is_ioport_supported() {
	return kh_is_ioport_supported();
}

PBOS_API void io_write_ioport8(uint16_t port, uint8_t data) {
	kh_write_ioport8(port, data);
}

PBOS_API uint8_t io_read_ioport8(uint16_t port) {
	return kh_read_ioport8(port);
}

PBOS_API void io_write_ioport16(uint16_t port, uint16_t data) {
	kh_write_ioport16(port, data);
}

PBOS_API uint16_t io_read_ioport16(uint16_t port) {
	return kh_read_ioport16(port);
}

PBOS_API void io_write_ioport32(uint16_t port, uint32_t data) {
	kh_write_ioport32(port, data);
}

PBOS_API uint32_t io_read_ioport32(uint16_t port) {
	return kh_read_ioport32(port);
}
