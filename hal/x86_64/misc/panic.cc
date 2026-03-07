#include <arch/x86_64/irq.h>
#include <arch/x86_64/misc.h>
#include <pbos/km/logger.h>
#include <pbos/km/panic.h>

PBOS_EXTERN_C_BEGIN

PBOS_NORETURN void _km_panic_halt();

static bool _panicked = false;
bool km_is_panicked() {
	return _panicked;
}

PBOS_NORETURN void km_panic(const char *str, ...) {
	if (_panicked)
		goto panicked;

	_panicked = true;

	va_list args;
	va_start(args, str);
	klog_printf(">> PANIC <<");
	klog_vprintf(str, args);
	va_end(args);
panicked:
	_km_panic_halt();
	// Bochs/Old QEMU
	arch_out16(0xb004, 0x2000);
	// QEMU
	arch_out16(0x0604, 0x2000);
}

PBOS_EXTERN_C_END
