#include "error.h"

#include <arch/i386/debug/bochs.h>
#include <arch/i386/misc.h>
#include <stdarg.h>
#include <string.h>

#include <hal/i386/display/vga.h>

static char _boot_errbuf[256] = { 0 };

const char* boot_geterr() {
	return _boot_errbuf;
}

void boot_seterr(const char* errstr) {
	strncpy(_boot_errbuf, errstr, 255);
}

void boot_panic(const char* msg, ...) {
	va_list args;
	va_start(args, msg);

	vga_setcolor(CHAR_COLOR_RED | CHAR_COLOR_LIGHT, CHAR_COLOR_BLACK);
	vga_vprintf(msg, args);

	va_end(args);

	arch_halt();
}
