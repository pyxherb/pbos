#include <common/format.h>
#include <common/mathex.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int sprintf(char *s, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);

	int retval = vsprintf(s, fmt, args);

	va_end(args);
	return retval;
}

int vsprintf(char* s, const char *fmt, va_list args) {
	// print_start, print_len: for print normal string fragment.
	// printed_len: Total generated string length.
	size_t print_start = 0, print_len = 0, printed_len = 0;
	for (size_t i = 0; i < strlen(fmt);) {
		if (fmt[i] == '%') {
			// Print previous scanned non-format-control character sequence.
			memcpy(s + printed_len, &fmt[print_start], print_len);

			printed_len += print_len;

			fmtctl_info_t info;

			// Parse format control character sequence.
			size_t parsed_len = fmtctl_parse(fmt + i, &info);

			// Check if the format control character sequence is invalid.
			if (!parsed_len)
				return -1;

			// Skip parsed area.
			i += parsed_len;
			print_start = i;
			print_len = 0;

			// Resolve the format control character sequence.
			switch (info.specifier) {
				case FMTCTL_SPECIFIER_DEC: {
					int arg = va_arg(args, int);

					if (arg > 0) {
						if (info.precision < digcount(arg))
							info.precision = digcount(arg);
						// Fill with space if the width has not reached.
						for (int j = 0; j < info.width - info.precision; ++j) {
							s[printed_len] = ' ';
							printed_len++;
						}

						for (int j = 0; j < info.precision - digcount(arg); ++j) {
							s[printed_len] = '0';
							printed_len++;
						}

						for (int j = digcount(arg); j > 0; j--) {
							s[printed_len] = getdigit(arg, j) + '0';
							printed_len++;
						}
					} else if (arg < 0) {
						s[printed_len] = '-';
						printed_len++;

						if (info.precision < digcount(arg))
							info.precision = digcount(arg);
						// Fill with space if the width has not reached.
						for (int j = 0; j < info.width - info.precision - 1; ++j) {
							s[printed_len] = ' ';
							printed_len++;
						}

						for (int j = 0; j < info.precision - digcount(arg) - 1; ++j) {
							s[printed_len] = '0';
							printed_len++;
						}

						for (int j = digcount(arg); j > 0; j--) {
							s[printed_len] = getdigit(arg, j) + '0';
							printed_len++;
						}
					} else {
						// Fill with space if the width has not reached.
						for (int j = 0; j < info.width - info.precision; ++j) {
							s[printed_len] = ' ';
							printed_len++;
						}

						for (int j = 0; j < info.precision; ++j) {
							s[printed_len] = '0';
							printed_len++;
						}

						s[printed_len] = '0';
					}
					break;
				}
				case FMTCTL_SPECIFIER_OCT: {
					unsigned int arg = va_arg(args, unsigned int);

					if (info.precision < odigcount(arg))
						info.precision = odigcount(arg);
					// Fill with space if the width has not reached.
					for (int j = 0; j < info.width - info.precision; ++j) {
						s[printed_len] = ' ';
						printed_len++;
					}

					for (int j = 0; j < info.precision - odigcount(arg); ++j) {
						s[printed_len] = '0';
						printed_len++;
					}

					for (int j = odigcount(arg); j > 0; j--) {
						s[printed_len] = getodigit(arg, j) + '0';
						printed_len++;
					}
					break;
				}
				case FMTCTL_SPECIFIER_HEX: {
					unsigned int arg = va_arg(args, unsigned int);

					if (info.precision < xdigcount(arg))
						info.precision = xdigcount(arg);
					// Fill with space if the width has not reached.
					for (int j = 0; j < info.width - info.precision; ++j) {
						s[printed_len] = ' ';
						printed_len++;
					}

					for (int j = 0; j < info.precision - xdigcount(arg); ++j) {
						s[printed_len] = '0';
						printed_len++;
					}

					for (int j = xdigcount(arg); j > 0; j--) {
						int digit = getxdigit(arg, j);
						if (digit > 9)
							s[printed_len] = digit - 10 + 'a';
						else
							s[printed_len] = digit + '0';
						printed_len++;
					}
					break;
				}
				case FMTCTL_SPECIFIER_UNSIGNED:
				case FMTCTL_SPECIFIER_LUNSIGNED: {
					unsigned int arg = va_arg(args, unsigned int);

					if (info.precision < udigcount(arg))
						info.precision = udigcount(arg);
					// Fill with space if the width has not reached.
					for (int j = 0; j < info.width - info.precision; ++j) {
						s[printed_len] = ' ';
						printed_len++;
					}

					for (int j = 0; j < info.precision - udigcount(arg); ++j) {
						s[printed_len] = '0';
						printed_len++;
					}

					for (int j = udigcount(arg); j > 0; j--) {
						s[printed_len] = getudigit(arg, j) + '0';
						printed_len++;
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
						s[printed_len] = ' ';
						printed_len++;
					}

					memcpy(s + printed_len, arg, strlen(arg));

					printed_len += strlen(arg);
					break;
				}
				case FMTCTL_SPECIFIER_CHAR: {
					char arg = va_arg(args, int);

					for (int j = 0; j < info.width - 1; ++j) {
						s[printed_len] = ' ';
						printed_len++;
					}

					s[printed_len] = arg;

					printed_len++;
					break;
				}
				case FMTCTL_SPECIFIER_PTR: {
					unsigned int arg = va_arg(args, unsigned int);

					if (info.precision < xdigcount(arg))
						info.precision = xdigcount(arg);
					// Fill with space if the width has not reached.
					for (int j = 0; j < info.width - 8 - 2; ++j) {
						s[printed_len] = ' ';
						printed_len++;
					}

					s[printed_len] = '0';
					printed_len++;
					s[printed_len] = 'x';
					printed_len++;

					for (int j = 0; j < 8 - xdigcount(arg); ++j) {
						s[printed_len] = '0';
						printed_len++;
					}

					for (int j = xdigcount(arg); j > 0; j--) {
						int digit = getxdigit(arg, j);
						if (digit > 9)
							s[printed_len] = digit - 10 + 'a';
						else
							s[printed_len] = digit + '0';
						printed_len++;
					}
					break;
				}
				case FMTCTL_SPECIFIER_LLUNSIGNED: {
					unsigned long long arg = va_arg(args, unsigned long long);

					if (info.precision < lludigcount(arg))
						info.precision = lludigcount(arg);
					// Fill with space if the width has not reached.
					for (int j = 0; j < info.width - info.precision; ++j) {
						s[printed_len] = ' ';
						printed_len++;
					}

					for (int j = 0; j < info.precision - lludigcount(arg); ++j) {
						s[printed_len] = '0';
						printed_len++;
					}

					for (int j = lludigcount(arg); j > 0; j--) {
						s[printed_len] = getlludigit(arg, j) + '0';
						printed_len++;
					}
					break;
				}
				case FMTCTL_SPECIFIER_PERCENT:
					s[printed_len] = '%';
					printed_len++;
					break;
			}
		} else
			print_len++, i++;
	}

	strcpy(s + printed_len, &fmt[print_start]);
	printed_len += print_len;

	return printed_len;
}
