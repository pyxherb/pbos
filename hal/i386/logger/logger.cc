#include <common/format.h>
#include <hal/i386/display/vga.h>
#include <hal/i386/logger.h>
#include <pbos/hal/spinlock.h>
#include <string.h>
#include <pbos/hal/irq.hh>

PBOS_EXTERN_C_BEGIN

klog_logger_t hn_active_logger;

hal_spinlock_t hn_logger_spinlock = HAL_SPINLOCK_UNLOCKED;

static size_t _vga_logger(uint16_t mode, const void *arg, va_list vargs);

void hn_klog_init() {
	klog_set_logger(klog_get_default_logger());

	kd_printf("Initialized kernel logger\n");
}

void klog_set_logger(klog_logger_t logger) {
	(hn_active_logger = logger)(KLOG_MODE_INIT, NULL, NULL);
}
klog_logger_t klog_get_logger() {
	return hn_active_logger;
}
klog_logger_t klog_get_default_logger() {
	return _vga_logger;
}

void klog_vprintf(const char *str, va_list args) {
	io::irq_disable_lock irq_lock;
	hal_spinlock_lock(&hn_logger_spinlock);
	hn_active_logger(KLOG_MODE_PRINTFMT, str, args);
	hal_spinlock_unlock(&hn_logger_spinlock);
}

void klog_printf(const char *str, ...) {
	va_list args;
	va_start(args, str);

	klog_vprintf(str, args);

	va_end(args);
}

void klog_putc(char ch) {
	char s[2] = { ch, '\0' };
	hn_active_logger(KLOG_MODE_PRINT, s, NULL);
}

void klog_puts(const char *str) {
	hn_active_logger(KLOG_MODE_PRINT, str, NULL);
	klog_putc('\n');
}

bool klog_is_capable(uint16_t id) {
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
			switch ((uintptr_t)arg) {
				case KLOG_CAP_COLOR:
					return true;
				default:
					return false;
			}
		case KLOG_MODE_EXCALL:
			switch ((uintptr_t)arg) {
				case KLOG_EXCALL_COLOR: {
					vga_setcolor((uint8_t)va_arg(vargs, uint32_t), (uint8_t)va_arg(vargs, uint32_t));
					break;
				}
			}

			break;
	}

	return 0;
}

PBOS_EXTERN_C_END
