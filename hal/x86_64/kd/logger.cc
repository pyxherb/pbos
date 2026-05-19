#include <arch/x86_64/io.h>
#include <fstdc-utils/format.h>
#include <fstdc-utils/mathex.h>
#include <pbos/hal/spinlock.h>
#include <string.h>
#include <hal/x86_64/logger.hh>
// #include <pbos/hal/irq.hh>

PBOS_EXTERN_C_BEGIN

size_t qemu_console_logger_print(const char *s, size_t len) {
	for (size_t i = 0; i < len; ++i) {
		if (s[i] == '\n')
			arch_out8(0xe9, '\r');
		arch_out8(0xe9, s[i]);
	}
	return 0;
}
size_t qemu_console_logger_putchar(char s) {
	if (s == '\n')
		arch_out8(0xe9, '\r');
	arch_out8(0xe9, s);
	return 0;
}
void qemu_console_logger_deinit() {
	return;
}
bool qemu_console_logger_query_cap(uint32_t cap) {
	return false;
}

kd_logger_t hn_qemu_logger = {
	.print = qemu_console_logger_print,
	.putchar = qemu_console_logger_putchar,
	.deinit = qemu_console_logger_deinit,
	.query_cap = qemu_console_logger_query_cap
};

kd_logger_t *kd_default_logger() {
	return &hn_qemu_logger;
}

PBOS_EXTERN_C_END
