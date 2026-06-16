#include <fstdc-utils/format.h>
#include <fstdc-utils/mathex.h>
#include <pbos/hal/spinlock.h>
#include <pbos/kd/logger.h>
#include <string.h>

PBOS_EXTERN_C_BEGIN

kd_logger_t *hali_active_logger;

hal_spinlock_t hali_logger_spinlock = HAL_SPINLOCK_UNLOCKED;

void kd_set_logger(kd_logger_t *logger) {
	hali_active_logger = logger;
}
kd_logger_t *kd_get_logger() {
	return hali_active_logger;
}

int do_vprintf(kd_logger_t *logger, const char *s, va_list args) {
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
					switch (info.length) {
						case FMTCTL_LENGTH_HALF:
						case FMTCTL_LENGTH_NONE: {
							int arg = va_arg(args, int);

							int num_digits = digcount(arg);

							if (arg > 0) {
								if (info.precision < num_digits)
									info.precision = num_digits;
								// Fill with space if the width has not reached.
								for (int j = 0; j < info.width - info.precision; ++j) {
									logger->putchar(' ');
									len_printed++;
								}

								for (int j = 0; j < info.precision - num_digits; ++j) {
									logger->putchar('0');
									len_printed++;
								}

								for (int j = num_digits; j > 0; j--) {
									int digit = getdigit(arg, j);
									logger->putchar(digit + '0');
									len_printed++;
								}
							} else if (arg < 0) {
								logger->putchar('-');
								len_printed++;

								if (info.precision < num_digits)
									info.precision = num_digits;
								// Fill with space if the width has not reached.
								for (int j = 0; j < info.width - info.precision - 1; ++j) {
									logger->putchar(' ');
									len_printed++;
								}

								for (int j = 0; j < info.precision - num_digits - 1; ++j) {
									logger->putchar('0');
									len_printed++;
								}

								arg = -arg;

								for (int j = num_digits; j > 0; j--) {
									int digit = getdigit(arg, j);
									logger->putchar(digit + '0');
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
								len_printed++;
							}
							break;
						}
						case FMTCTL_LENGTH_LONG:
						case FMTCTL_LENGTH_LONG_LONG: {
							long long arg = va_arg(args, long long);

							long long num_digits = lldigcount(arg);

							if (arg > 0) {
								if (info.precision < num_digits)
									info.precision = num_digits;
								// Fill with space if the width has not reached.
								for (int j = 0; j < info.width - info.precision; ++j) {
									logger->putchar(' ');
									len_printed++;
								}

								for (int j = 0; j < info.precision - num_digits; ++j) {
									logger->putchar('0');
									len_printed++;
								}

								for (int j = num_digits; j > 0; j--) {
									int digit = getlldigit(arg, j);
									logger->putchar(digit + '0');
									len_printed++;
								}
							} else if (arg < 0) {
								logger->putchar('-');
								len_printed++;

								if (info.precision < num_digits)
									info.precision = num_digits;
								// Fill with space if the width has not reached.
								for (int j = 0; j < info.width - info.precision - 1; ++j) {
									logger->putchar(' ');
									len_printed++;
								}

								for (int j = 0; j < info.precision - num_digits - 1; ++j) {
									logger->putchar('0');
									len_printed++;
								}

								arg = -arg;

								for (int j = num_digits; j > 0; j--) {
									int digit = getlldigit(arg, j);
									logger->putchar(digit + '0');
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
								len_printed++;
							}
							break;
						}
					}
					break;
				}
				case FMTCTL_SPECIFIER_OCT: {
					switch (info.length) {
						case FMTCTL_LENGTH_HALF:
						case FMTCTL_LENGTH_NONE: {
							unsigned int arg = va_arg(args, unsigned int);

							int num_digits = odigcount(arg);
							if (info.precision < num_digits)
								info.precision = num_digits;
							// Fill with space if the width has not reached.
							for (int j = 0; j < info.width - info.precision; ++j) {
								logger->putchar(' ');
								len_printed++;
							}

							for (int j = 0; j < info.precision - num_digits; ++j) {
								logger->putchar('0');
								len_printed++;
							}

							for (int j = num_digits; j > 0; j--) {
								int digit = getodigit(arg, j);
								logger->putchar(digit + '0');
								len_printed++;
							}
						}
						case FMTCTL_LENGTH_LONG:
						case FMTCTL_LENGTH_LONG_LONG: {
							// TODO: Implement it.
							break;
						}
					}
					break;
				}
				case FMTCTL_SPECIFIER_HEX: {
					switch (info.length) {
						case FMTCTL_LENGTH_HALF:
						case FMTCTL_LENGTH_NONE: {
							unsigned int arg = va_arg(args, unsigned int);

							int num_digits = xdigcount(arg);
							if (info.precision < num_digits)
								info.precision = num_digits;
							// Fill with space if the width has not reached.
							for (int j = 0; j < info.width - info.precision; ++j) {
								logger->putchar(' ');
								len_printed++;
							}

							for (int j = 0; j < info.precision - num_digits; ++j) {
								logger->putchar('0');
								len_printed++;
							}

							for (int j = num_digits; j > 0; j--) {
								int digit = getxdigit(arg, j);
								if (digit > 9)
									logger->putchar(digit - 10 + 'a');
								else
									logger->putchar(digit + '0');
								len_printed++;
							}
							break;
						}
						case FMTCTL_LENGTH_LONG:
						case FMTCTL_LENGTH_LONG_LONG: {
							unsigned long long arg = va_arg(args, unsigned long long);

							int num_digits = llxdigcount(arg);
							if (info.precision < num_digits)
								info.precision = num_digits;
							// Fill with space if the width has not reached.
							for (int j = 0; j < info.width - info.precision; ++j) {
								logger->putchar(' ');
								len_printed++;
							}

							for (int j = 0; j < info.precision - num_digits; ++j) {
								logger->putchar('0');
								len_printed++;
							}

							for (int j = num_digits; j > 0; j--) {
								int digit = getllxdigit(arg, j);
								if (digit > 9)
									logger->putchar(digit - 10 + 'a');
								else
									logger->putchar(digit + '0');
								len_printed++;
							}
							break;
						}
					}
					break;
				}
				case FMTCTL_SPECIFIER_UNSIGNED: {
					switch (info.length) {
						case FMTCTL_LENGTH_HALF:
						case FMTCTL_LENGTH_NONE: {
							unsigned int arg = va_arg(args, unsigned int);

							int num_digits = udigcount(arg);
							if (info.precision < num_digits)
								info.precision = num_digits;
							// Fill with space if the width has not reached.
							for (int j = 0; j < info.width - info.precision; ++j) {
								logger->putchar(' ');
								len_printed++;
							}

							for (int j = 0; j < info.precision - num_digits; ++j) {
								logger->putchar('0');
								len_printed++;
							}

							for (int j = num_digits; j > 0; j--) {
								int digit = getudigit(arg, j);
								logger->putchar(digit + '0');
								len_printed++;
							}
							break;
						}
						case FMTCTL_LENGTH_LONG: {
							unsigned long arg = va_arg(args, unsigned int);

							int num_digits = ludigcount(arg);
							if (info.precision < num_digits)
								info.precision = num_digits;
							// Fill with space if the width has not reached.
							for (int j = 0; j < info.width - info.precision; ++j) {
								logger->putchar(' ');
								len_printed++;
							}

							for (int j = 0; j < info.precision - num_digits; ++j) {
								logger->putchar('0');
								len_printed++;
							}

							for (int j = num_digits; j > 0; j--) {
								int digit = getludigit(arg, j);
								logger->putchar(digit + '0');
								len_printed++;
							}
							break;
						}
						case FMTCTL_LENGTH_LONG_LONG: {
							unsigned long long arg = va_arg(args, unsigned int);

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

							if (!arg) {
								logger->putchar('0');
								len_printed++;
							} else
								while (arg) {
									logger->putchar((arg % 10) + '0');
									arg /= 10;
									len_printed++;
								}
							break;
						}
						default:
							break;
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
					if (sizeof(void *) == sizeof(uint64_t)) {
						uint64_t arg = va_arg(args, uint64_t);

						int num_digits = llxdigcount(arg);
						if (info.precision < num_digits)
							info.precision = num_digits;
						// Fill with space if the width has not reached.
						for (int j = 0; j < info.width - 16 - 2; ++j) {
							logger->putchar(' ');
							len_printed++;
						}

						logger->print("0x", 2);

						for (int j = 0; j < 16 - num_digits; ++j) {
							logger->putchar('0');
							len_printed++;
						}

						for (int j = num_digits; j > 0; j--) {
							int digit = getllxdigit(arg, j);
							if (digit > 9)
								logger->putchar(digit - 10 + 'a');
							else
								logger->putchar(digit + '0');
							len_printed++;
						}
					} else {
						uint32_t arg = va_arg(args, uint32_t);

						int num_digits = llxdigcount(arg);
						if (info.precision < num_digits)
							info.precision = num_digits;
						// Fill with space if the width has not reached.
						for (int j = 0; j < info.width - 8 - 2; ++j) {
							logger->putchar(' ');
							len_printed++;
						}

						logger->print("0x", 2);

						for (int j = 0; j < 8 - num_digits; ++j) {
							logger->putchar('0');
							len_printed++;
						}

						for (int j = num_digits; j > 0; j--) {
							int digit = getllxdigit(arg, j);
							if (digit > 9)
								logger->putchar(digit - 10 + 'a');
							else
								logger->putchar(digit + '0');
							len_printed++;
						}
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
	// io::LocalIrqLock irq_lock;
	hal_lock_spinlock(&hali_logger_spinlock);
	do_vprintf(hali_active_logger, str, args);
	hal_unlock_spinlock(&hali_logger_spinlock);
}

PBOS_API void klog_printf(const char *str, ...) {
	va_list args;
	va_start(args, str);

	klog_vprintf(str, args);

	va_end(args);
}

void klog_putc(char ch) {
	hali_active_logger->putchar(ch);
}

void klog_puts(const char *str) {
	hali_active_logger->print(str, strlen(str));
	klog_putc('\n');
}

PBOS_EXTERN_C_END
