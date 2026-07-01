#include <pbos/io/ioport.h>
#include <arch/x86_64/io.h>

PBOS_API bool kh_is_ioport_supported() {
	return true;
}

PBOS_API void kh_write_ioport8(uint16_t port, uint8_t data) {
	arch_out8(port, data);
}
PBOS_API uint8_t kh_read_ioport8(uint16_t port) {
	return arch_in8(port);
}
PBOS_API void kh_write_ioport16(uint16_t port, uint16_t data) {
	arch_out16(port, data);
}
PBOS_API uint16_t kh_read_ioport16(uint16_t port){
	return arch_in16(port);
}
PBOS_API void kh_write_ioport32(uint16_t port, uint32_t data){
	arch_out32(port, data);
}
PBOS_API uint32_t kh_read_ioport32(uint16_t port){
	return arch_in32(port);
}
