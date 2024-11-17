#include <arch/i386/int.h>
#include <arch/i386/misc.h>
#include <hal/i386/display/vga.h>
#include <pbos/km/panic.h>

PB_NORETURN void _km_panic_halt();

static bool _panicked = false;
bool km_ispanicked() {
	return _panicked;
}

PB_NORETURN void km_panic(const char *str, ...) {
	if (_panicked)
		goto panicked;

	_panicked = true;

	va_list args;
	va_start(args, str);
	vga_setcolor(CHAR_COLOR_RED | CHAR_COLOR_LIGHT, CHAR_COLOR_BLACK);
	vga_puts(">> PANIC <<");
	vga_vprintf(str, args);
	va_end(args);
panicked:
	_km_panic_halt();
	// Bochs/Old QEMU
	arch_out16(0xb004, 0x2000);
	// QEMU
	arch_out16(0x0604, 0x2000);
}
