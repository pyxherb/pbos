#ifndef _HAL_I386_DISPLAY_VGA_H_
#define _HAL_I386_DISPLAY_VGA_H_

#include <pbos/common.h>
#include <stdarg.h>
#include <stddef.h>

#define VGA_TEXTDISP_BUFFER 0x000b8000
#define VGA_TEXTDISP_BUFFER_TOP 0x000bffff
#define VGA_TEXTDISP_BUFFER_SIZE 0x8000

#define VGA_TEXTDISP_WIDTH 80
#define VGA_TEXTDISP_HEIGHT 25

#define CHAR_COLOR_BLACK 0x00
#define CHAR_COLOR_BLUE 0x01
#define CHAR_COLOR_GREEN 0x02
#define CHAR_COLOR_CYAN 0x03
#define CHAR_COLOR_RED 0x04
#define CHAR_COLOR_PURPLE 0x05
#define CHAR_COLOR_YELLOW 0x06
#define CHAR_COLOR_WHITE 0x07
#define CHAR_COLOR_LIGHT 0x08

#define CURSOR_POS_Y(lp) ((lp) / VGA_TEXTDISP_WIDTH)
#define CURSOR_POS_X(lp) ((lp) % VGA_TEXTDISP_WIDTH)

void vga_clear();

void vga_hidecur();
void vga_showcur();

void vga_setpos(uint8_t x, uint8_t y);
uint16_t vga_getpos();

void vga_chwrite(unsigned char ch, uint8_t fgcolor, uint8_t bgcolor, uint8_t x, uint8_t y);
void vga_scroll();

void vga_strnwrite(const char *s, size_t n, uint8_t fgcolor, uint8_t bgcolor);

void vga_setcolor(uint8_t fgcolor, uint8_t bgcolor);

void vga_putcchar(char c, uint8_t fgcolor, uint8_t bgcolor);

int vga_putchar(int c);
int vga_puts(const char *s);
int vga_vprintf(const char *s, va_list args);
int vga_printf(const char *s, ...);

#endif
