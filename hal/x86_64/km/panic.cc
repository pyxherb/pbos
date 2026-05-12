#include <arch/x86_64/irq.h>
#include <arch/x86_64/misc.h>
#include <pbos/kd/logger.h>
#include <pbos/kh/km/panic.h>

PBOS_EXTERN_C_BEGIN

PBOS_NORETURN void kh_panic_halt() {
	arch_hlt();
	// Bochs/Old QEMU
	arch_out16(0xb004, 0x2000);
	// QEMU
	arch_out16(0x0604, 0x2000);
}

PBOS_EXTERN_C_END
