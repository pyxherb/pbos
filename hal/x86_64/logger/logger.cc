#include <arch/x86_64/io.h>
#include <common/format.h>
#include <common/mathex.h>
#include <hal/x86_64/logger.hh>
#include <pbos/hal/spinlock.h>
#include <string.h>
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

klog_logger_t hn_qemu_logger = {
	.print = qemu_console_logger_print,
	.putchar = qemu_console_logger_putchar,
	.deinit = qemu_console_logger_deinit,
	.query_cap = qemu_console_logger_query_cap
};
klog_logger_t *hn_active_logger;

hal_spinlock_t hn_logger_spinlock = HAL_SPINLOCK_UNLOCKED;

void hn_klog_init() {
	klog_set_logger(klog_get_default_logger());

	kd_printf("Initialized kernel logger\n");
}

void klog_set_logger(klog_logger_t *logger) {
	hn_active_logger = logger;
}
klog_logger_t *klog_get_logger() {
	return hn_active_logger;
}
klog_logger_t *klog_get_default_logger() {
	return &hn_qemu_logger;
}

int do_vprintf(klog_logger_t *logger, const char *s, va_list args) {
	// off_start, len_print: for print normal string fragment.
	// len_printed: Total generated string length.
	size_t off_start = 0, len_print = 0, len_printed = 0;
	size_t len = strlen(s);
	for (size_t i = 0; i < len;) {
		if (s[i] == '%') {
			// Print previous scanned non-format-control character sequence.
			logger->print(&s[off_start], len_print);

			len_printed += len_print;

			fmtctl_info_t info;

			// Parse format control character sequence.
			size_t parsed_len = fmtctl_parse(s + i, &info);

			// Check if the format control character sequence is invalid.
			if (!parsed_len)
				return -1;

			// Skip parsed area.
			off_start = (i += parsed_len);
			len_print = 0;

			// Resolve the format control character sequence.
			switch (info.specifier) {
				case FMTCTL_SPECIFIER_DEC: {
					int arg = va_arg(args, int);

					if (arg > 0) {
						if (info.precision < digcount(arg))
							info.precision = digcount(arg);
						// Fill with space if the width has not reached.
						for (int j = 0; j < info.width - info.precision; ++j) {
							logger->putchar(' ');
							len_printed++;
						}

						for (int j = 0; j < info.precision - digcount(arg); ++j) {
							logger->putchar('0');
							len_printed++;
						}

						for (int j = digcount(arg); j > 0; j--) {
							logger->putchar(getdigit(arg, j) + '0');
							len_printed++;
						}
					} else if (arg < 0) {
						logger->putchar('-');
						len_printed++;

						if (info.precision < digcount(arg))
							info.precision = digcount(arg);
						// Fill with space if the width has not reached.
						for (int j = 0; j < info.width - info.precision - 1; ++j) {
							logger->putchar(' ');
							len_printed++;
						}

						for (int j = 0; j < info.precision - digcount(arg) - 1; ++j) {
							logger->putchar('0');
							len_printed++;
						}

						for (int j = digcount(arg); j > 0; j--) {
							logger->putchar(getdigit(arg, j) + '0');
							len_printed++;
						}
					} else {
						// Fill with space if the width has not reached.
						for (int j = 0; j < info.width - info.precision; ++j) {
							logger->putchar(' ');
							len_printed++;
						}

						for (int j = 0; j < info.precision; ++j) {
							logger->putchar('0');
							len_printed++;
						}

						logger->putchar('0');
					}
					break;
				}
				case FMTCTL_SPECIFIER_OCT: {
					uint32_t arg = va_arg(args, unsigned int);

					if (info.precision < odigcount(arg))
						info.precision = odigcount(arg);
					// Fill with space if the width has not reached.
					for (int j = 0; j < info.width - info.precision; ++j) {
						logger->putchar(' ');
						len_printed++;
					}

					for (int j = 0; j < info.precision - odigcount(arg); ++j) {
						logger->putchar('0');
						len_printed++;
					}

					for (int j = odigcount(arg); j > 0; j--) {
						logger->putchar(getodigit(arg, j) + '0');
						len_printed++;
					}
					break;
				}
				case FMTCTL_SPECIFIER_HEX: {
					uint32_t arg = va_arg(args, unsigned int);

					if (info.precision < xdigcount(arg))
						info.precision = xdigcount(arg);
					// Fill with space if the width has not reached.
					for (int j = 0; j < info.width - info.precision; ++j) {
						logger->putchar(' ');
						len_printed++;
					}

					for (int j = 0; j < info.precision - xdigcount(arg); ++j) {
						logger->putchar('0');
						len_printed++;
					}

					for (int j = xdigcount(arg); j > 0; j--) {
						int digit = getxdigit(arg, j);
						if (digit > 9)
							logger->putchar(digit - 10 + 'a');
						else
							logger->putchar(digit + '0');
						len_printed++;
					}
					break;
				}
				case FMTCTL_SPECIFIER_UNSIGNED:
				case FMTCTL_SPECIFIER_LUNSIGNED: {
					uint32_t arg = va_arg(args, unsigned int);

					if (info.precision < udigcount(arg))
						info.precision = udigcount(arg);
					// Fill with space if the width has not reached.
					for (int j = 0; j < info.width - info.precision; ++j) {
						logger->putchar(' ');
						len_printed++;
					}

					for (int j = 0; j < info.precision - udigcount(arg); ++j) {
						logger->putchar('0');
						len_printed++;
					}

					for (int j = udigcount(arg); j > 0; j--) {
						logger->putchar(getudigit(arg, j) + '0');
						len_printed++;
					}
					break;
				}
				// TODO: Implement floating-point functions.
				case FMTCTL_SPECIFIER_FLOAT:
					break;
				case FMTCTL_SPECIFIER_FLOAT_EXPONENTIAL:
					break;
				case FMTCTL_SPECIFIER_FLOAT_TRIMMED:
					break;
				case FMTCTL_SPECIFIER_STR: {
					const char *arg = va_arg(args, const char *);

					for (int j = info.width - strlen(arg); j > 0; j--) {
						logger->putchar(' ');
						len_printed++;
					}

					logger->print(arg, strlen(arg));

					len_printed += strlen(arg);
					break;
				}
				case FMTCTL_SPECIFIER_CHAR: {
					char arg = va_arg(args, int);

					for (int j = 0; j < info.width - 1; ++j) {
						logger->putchar(' ');
						len_printed++;
					}

					logger->putchar(arg);

					len_printed++;
					break;
				}
				case FMTCTL_SPECIFIER_PTR: {
					uint64_t arg = va_arg(args, uint64_t);

					if (info.precision < llxdigcount(arg))
						info.precision = llxdigcount(arg);
					// Fill with space if the width has not reached.
					for (int j = 0; j < info.width - 16 - 2; ++j) {
						logger->putchar(' ');
						len_printed++;
					}

					logger->print("0x", 2);

					for (int j = 0; j < 16 - llxdigcount(arg); ++j) {
						logger->putchar('0');
						len_printed++;
					}

					for (int j = llxdigcount(arg); j > 0;) {
						int digit = getllxdigit(arg, j--);
						if (digit > 9)
							logger->putchar(digit - 10 + 'a');
						else
							logger->putchar(digit + '0');
						len_printed++;
					}
					break;
				}
				case FMTCTL_SPECIFIER_LLUNSIGNED: {
					uint64_t arg = va_arg(args, uint64_t);

					if (info.precision < lludigcount(arg))
						info.precision = lludigcount(arg);
					// Fill with space if the width has not reached.
					for (int j = 0; j < info.width - info.precision; ++j) {
						logger->putchar(' ');
						len_printed++;
					}

					for (int j = 0; j < info.precision - lludigcount(arg); ++j) {
						logger->putchar('0');
						len_printed++;
					}

					for (int j = lludigcount(arg); j > 0; j--) {
						logger->putchar(getlludigit(arg, j) + '0');
						len_printed++;
					}
					break;
				}
				case FMTCTL_SPECIFIER_PERCENT:
					logger->putchar('%');
					len_printed++;
					break;
			}
		} else
			len_print++, i++;
	}

	len_printed += len_print;
	logger->print(&s[off_start], len_print);

	return len_printed;
}

void klog_vprintf(const char *str, va_list args) {
	// io::irq_disable_lock irq_lock;
	hal_spinlock_lock(&hn_logger_spinlock);
	do_vprintf(hn_active_logger, str, args);
	hal_spinlock_unlock(&hn_logger_spinlock);
}

void klog_printf(const char *str, ...) {
	va_list args;
	va_start(args, str);

	klog_vprintf(str, args);

	va_end(args);
}

void klog_putc(char ch) {
	hn_active_logger->putchar(ch);
}

void klog_puts(const char *str) {
	hn_active_logger->print(str, strlen(str));
	klog_putc('\n');
}

bool klog_is_capable(uint16_t id) {
	return hn_active_logger->query_cap(id);
}

PBOS_EXTERN_C_END
