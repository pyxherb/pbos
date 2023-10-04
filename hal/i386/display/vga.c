#include "vga.h"

#include <arch/i386/port.h>
#include <common/format.h>
#include <common/mathex.h>
#include <stdint.h>
#include <string.h>

static uint8_t _cur_bgcolor = CHAR_COLOR_BLACK;
static uint8_t _cur_fgcolor = CHAR_COLOR_WHITE;

/// @brief Clear content of the screen buffer.
void vga_clear() {
	memset((uint8_t *)VGA_TEXTDISP_BUFFER, 0, VGA_TEXTDISP_WIDTH * VGA_TEXTDISP_HEIGHT * 2);
	vga_setpos(0, 0);
}

/// @brief Set position of the cursor.
///
/// @param x X position to set.
/// @param y Y position to set.
void vga_setpos(uint8_t x, uint8_t y) {
	uint16_t offset = y * VGA_TEXTDISP_WIDTH + x;
	// Higher part of the position.
	arch_out8(0x03d4, 0x0e);
	arch_out8(0x03d5, offset >> 8);

	// Lower part of the position.
	arch_out8(0x03d4, 0x0f);
	arch_out8(0x03d5, offset & 0xff);
}

/// @brief Get position of the cursor.
///
/// @return Linear position of the cursor.
uint16_t vga_getpos() {
	uint16_t offset = 0;

	// Position high.
	arch_out8(0x03d4, 0x0e);
	offset = (arch_in8(0x03d5) << 8);
	// Position low.
	arch_out8(0x03d4, 0x0f);
	offset |= arch_in8(0x03d5);

	return offset;
}

void vga_strnwrite(const char *s, size_t n, uint8_t fgcolor, uint8_t bgcolor) {
	for (size_t i = 0; i < n; ++i) vga_putcchar(s[i], fgcolor, bgcolor);
}

void vga_setcolor(uint8_t fgcolor, uint8_t bgcolor) {
	_cur_fgcolor = fgcolor;
	_cur_bgcolor = bgcolor;
}

void vga_putcchar(char c, uint8_t fgcolor, uint8_t bgcolor) {
	uint16_t pos = vga_getpos();
	switch (c) {
		case '\a':
			// Ignored
			break;
		case '\n':
			vga_setpos(0, CURSOR_POS_Y(pos) + 1);
			break;
		case '\t': {
			uint8_t x_diff = (CURSOR_POS_X(pos)) % 4;

			if (x_diff)
				// Align to 4 characters.
				vga_setpos(CURSOR_POS_X(pos) + (4 - x_diff), CURSOR_POS_Y(pos));
			else
				// Move forward 4 characters.
				vga_setpos(CURSOR_POS_X(pos) + 4, CURSOR_POS_Y(pos));
			break;
		}
		case '\b':
			// Back a character.
			if (CURSOR_POS_X(pos))
				vga_setpos(CURSOR_POS_X(pos) - 1, CURSOR_POS_Y(pos));
			break;
		case '\r':
			vga_setpos(0, CURSOR_POS_Y(pos));
			break;
		default:
			vga_chwrite(c, fgcolor, bgcolor, CURSOR_POS_X(pos), CURSOR_POS_Y(pos));
			vga_setpos(CURSOR_POS_X(pos) + 1, CURSOR_POS_Y(pos));
	}
	if (CURSOR_POS_Y(vga_getpos()) == VGA_TEXTDISP_HEIGHT) {
		vga_scroll();
		vga_setpos(CURSOR_POS_X(vga_getpos()), VGA_TEXTDISP_HEIGHT - 1);
	}
}

int vga_putchar(int c) {
	vga_putcchar((char)c, _cur_fgcolor, _cur_bgcolor);
	return c;
}

int vga_puts(const char *s) {
	const size_t len = strlen(s);
	vga_strnwrite(s, len, _cur_fgcolor, _cur_bgcolor);
	vga_putchar('\n');
	return len + 1;
}

int vga_vprintf(const char *s, va_list args) {
	// off_start, len_print: for print normal string fragment.
	// len_printed: Total generated string length.
	size_t off_start = 0, len_print = 0, len_printed = 0;
	size_t len = strlen(s);
	for (size_t i = 0; i < len;) {
		if (s[i] == '%') {
			// Print previous scanned non-format-control character sequence.
			vga_strnwrite(&s[off_start], len_print, _cur_fgcolor, _cur_bgcolor);

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
							vga_putchar(' ');
							len_printed++;
						}

						for (int j = 0; j < info.precision - digcount(arg); ++j) {
							vga_putchar('0');
							len_printed++;
						}

						for (int j = digcount(arg); j > 0; j--) {
							vga_putchar(getdigit(arg, j) + '0');
							len_printed++;
						}
					} else if (arg < 0) {
						vga_putchar('-');
						len_printed++;

						if (info.precision < digcount(arg))
							info.precision = digcount(arg);
						// Fill with space if the width has not reached.
						for (int j = 0; j < info.width - info.precision - 1; ++j) {
							vga_putchar(' ');
							len_printed++;
						}

						for (int j = 0; j < info.precision - digcount(arg) - 1; ++j) {
							vga_putchar('0');
							len_printed++;
						}

						for (int j = digcount(arg); j > 0; j--) {
							vga_putchar(getdigit(arg, j) + '0');
							len_printed++;
						}
					} else {
						// Fill with space if the width has not reached.
						for (int j = 0; j < info.width - info.precision; ++j) {
							vga_putchar(' ');
							len_printed++;
						}

						for (int j = 0; j < info.precision; ++j) {
							vga_putchar('0');
							len_printed++;
						}

						vga_putchar('0');
					}
					break;
				}
				case FMTCTL_SPECIFIER_OCT: {
					uint32_t arg = va_arg(args, unsigned int);

					if (info.precision < odigcount(arg))
						info.precision = odigcount(arg);
					// Fill with space if the width has not reached.
					for (int j = 0; j < info.width - info.precision; ++j) {
						vga_putchar(' ');
						len_printed++;
					}

					for (int j = 0; j < info.precision - odigcount(arg); ++j) {
						vga_putchar('0');
						len_printed++;
					}

					for (int j = odigcount(arg); j > 0; j--) {
						vga_putchar(getodigit(arg, j) + '0');
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
						vga_putchar(' ');
						len_printed++;
					}

					for (int j = 0; j < info.precision - xdigcount(arg); ++j) {
						vga_putchar('0');
						len_printed++;
					}

					for (int j = xdigcount(arg); j > 0; j--) {
						int digit = getxdigit(arg, j);
						if (digit > 9)
							vga_putchar(digit - 10 + 'a');
						else
							vga_putchar(digit + '0');
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
						vga_putchar(' ');
						len_printed++;
					}

					for (int j = 0; j < info.precision - udigcount(arg); ++j) {
						vga_putchar('0');
						len_printed++;
					}

					for (int j = udigcount(arg); j > 0; j--) {
						vga_putchar(getudigit(arg, j) + '0');
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
						vga_putchar(' ');
						len_printed++;
					}

					vga_strnwrite(arg, strlen(arg), _cur_fgcolor, _cur_bgcolor);

					len_printed += strlen(arg);
					break;
				}
				case FMTCTL_SPECIFIER_CHAR: {
					char arg = va_arg(args, int);

					for (int j = 0; j < info.width - 1; ++j) {
						vga_putchar(' ');
						len_printed++;
					}

					vga_putchar(arg);

					len_printed++;
					break;
				}
				case FMTCTL_SPECIFIER_PTR: {
					uint32_t arg = va_arg(args, unsigned int);

					if (info.precision < xdigcount(arg))
						info.precision = xdigcount(arg);
					// Fill with space if the width has not reached.
					for (int j = 0; j < info.width - 8 - 2; ++j) {
						vga_putchar(' ');
						len_printed++;
					}

					vga_strnwrite("0x", 2, _cur_fgcolor, _cur_bgcolor);

					for (int j = 0; j < 8 - xdigcount(arg); ++j) {
						vga_putchar('0');
						len_printed++;
					}

					for (int j = xdigcount(arg); j > 0;) {
						int digit = getxdigit(arg, j--);
						if (digit > 9)
							vga_putchar(digit - 10 + 'a');
						else
							vga_putchar(digit + '0');
						len_printed++;
					}
					break;
				}
				case FMTCTL_SPECIFIER_LLUNSIGNED: {
					uint64_t arg = va_arg(args, unsigned long long);

					if (info.precision < lludigcount(arg))
						info.precision = lludigcount(arg);
					// Fill with space if the width has not reached.
					for (int j = 0; j < info.width - info.precision; ++j) {
						vga_putchar(' ');
						len_printed++;
					}

					for (int j = 0; j < info.precision - lludigcount(arg); ++j) {
						vga_putchar('0');
						len_printed++;
					}

					for (int j = lludigcount(arg); j > 0; j--) {
						vga_putchar(getlludigit(arg, j) + '0');
						len_printed++;
					}
					break;
				}
				case FMTCTL_SPECIFIER_PERCENT:
					vga_putchar('%');
					len_printed++;
					break;
			}
		} else
			len_print++, i++;
	}

	len_printed += len_print;
	vga_strnwrite(&s[off_start], len_print, _cur_fgcolor, _cur_bgcolor);

	return len_printed;
}

int vga_printf(const char *s, ...) {
	va_list args;
	va_start(args, s);

	int printed_len = vga_vprintf(s, args);

	va_end(args);

	return printed_len;
}

/**
 * @brief Scroll one line.
 *
 */
void vga_scroll() {
	uint8_t *p_topline = (uint8_t *)(VGA_TEXTDISP_BUFFER + VGA_TEXTDISP_WIDTH * 2);
	size_t size = VGA_TEXTDISP_WIDTH * VGA_TEXTDISP_HEIGHT * 2;

	// Move all line of text forward 1 line.
	memcpy((uint8_t *)VGA_TEXTDISP_BUFFER, p_topline, size);
}

void vga_chwrite(unsigned char ch, uint8_t fgcolor, uint8_t bgcolor, uint8_t x, uint8_t y) {
	const uint8_t attrib = fgcolor | bgcolor << 4;
	uint8_t *dest = (uint8_t *)(VGA_TEXTDISP_BUFFER + (y * VGA_TEXTDISP_WIDTH + x) * 2);
	dest[0] = ch;
	dest[1] = attrib;
}
