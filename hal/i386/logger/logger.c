#include <common/format.h>
#include <hal/i386/display/vga.h>
#include <hal/i386/logger.h>
#include <string.h>

klog_logger_t hn_active_logger;

static size_t _vga_logger(uint16_t mode, const void *arg, va_list vargs);

void hn_klog_init() {
	klog_setlogger(klog_getdefault());

	kdprintf("Initialized kernel logger\n");
}

void klog_setlogger(klog_logger_t logger) {
	(hn_active_logger = logger)(KLOG_MODE_INIT, NULL, NULL);
}
klog_logger_t klog_getlogger() {
	return hn_active_logger;
}
klog_logger_t klog_getdefault() {
	return _vga_logger;
}

void kvprintf(const char *str, va_list args) {
	hn_active_logger(KLOG_MODE_PRINTFMT, str, args);
}

void kprintf(const char *str, ...) {
	va_list args;
	va_start(args, str);

	kvprintf(str, args);

	va_end(args);
}

void kputc(char ch) {
	char s[2] = { ch, '\0' };
	hn_active_logger(KLOG_MODE_PRINT, s, NULL);
}

void kputs(const char *str) {
	hn_active_logger(KLOG_MODE_PRINT, str, NULL);
	kputc('\n');
}

bool klog_iscapable(uint16_t id) {
	return hn_active_logger(KLOG_MODE_GETCAP, (const char *)id, NULL);
}

void klog_excall(uint32_t id, ...) {
	va_list args;
	va_start(args, id);

	hn_active_logger(KLOG_MODE_EXCALL, (void *)((unsigned long)id), args);

	va_end(args);
}

static size_t _vga_logger(uint16_t mode, const void *arg, va_list vargs) {
	switch (mode) {
		case KLOG_MODE_INIT:
			vga_clear();
			break;
		case KLOG_MODE_PRINT:
			vga_printf("%s", (const char *)arg);
			break;
		case KLOG_MODE_PRINTFMT:
			vga_vprintf((const char *)arg, vargs);
			break;
		case KLOG_MODE_GETCAP:
			switch ((uint16_t)arg) {
				case KLOG_CAP_COLOR:
					return true;
				default:
					return false;
			}
		case KLOG_MODE_EXCALL:
			switch ((uint16_t)arg) {
				case KLOG_EXCALL_COLOR: {
					vga_setcolor((uint8_t)va_arg(vargs, uint32_t), (uint8_t)va_arg(vargs, uint32_t));
					break;
				}
			}

			break;
	}

	return 0;
}
