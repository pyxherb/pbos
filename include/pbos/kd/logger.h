#ifndef _PBOS_KM_LOGGER_H_
#define _PBOS_KM_LOGGER_H_

#include <pbos/attribs.h>
#include <pbos/common.h>
#include <stdarg.h>

PBOS_EXTERN_C_BEGIN

enum {
	KLOG_CAP_COLOR = 0
};

enum {
	KD_LOG_LEVEL_DEBUG = 0,  // Debug information
	KD_LOG_LEVEL_INFO,	   // Information
	KD_LOG_LEVEL_NOTE,	   // Information that should be noticed by users
	KD_LOG_LEVEL_WARNING,	   // Warnings
	KD_LOG_LEVEL_ERROR,	   // Errors that do not cause the kernel to become unstable
	KD_LOG_LEVEL_FATAL,	   // Errors that may cause the kernel to become unstable
};

enum {
	KD_LOGGER_MODE_INIT = 0,	 // Initialize the logger.
	KD_LOGGER_MODE_GETCAP,	 // Check if the logger has specified capability.
	KD_LOGGER_MODE_PRINT,	 // Output plain text.
	KD_LOGGER_MODE_PRINTFMT,	 // Output formatted text.
	KD_LOGGER_MODE_EXCALL	 // Call an extended function.
};

typedef size_t (*kd_logger_print_t)(const char *s, size_t len);
typedef size_t (*kd_logger_putchar_t)(char s);
typedef void (*kd_logger_deinit_t)();
typedef bool (*kd_logger_query_cap_t)(uint32_t cap);
typedef struct _kd_logger_t {
	kd_logger_print_t print;
	kd_logger_putchar_t putchar;
	kd_logger_deinit_t deinit;
	kd_logger_query_cap_t query_cap;
} kd_logger_t;

/// @brief Set current logger for kernel logger system.
///
/// @param logger Logger to set.
void kd_set_logger(kd_logger_t *logger);

/// @brief Get default kernel logger.
///
/// @return The default kernel logger.
kd_logger_t *kd_default_logger();

/// @brief Get current kernel logger.
///
/// @return Current kernel logger.
kd_logger_t *kd_get_logger();

#define KLOG_EXCALL_COLOR 0x00000000

#define KLOG_EXCALL_PRIVATE_MIN 0xc0000000
#define KLOG_EXCALL_PRIVATE_MAX 0xffffffff

typedef struct _klog_color_exarg_t {
	uint8_t fg;
	uint8_t bg;
} klog_color_exarg_t;

void klog_vprintf(const char *str, va_list args) PBOS_FMTARG(printf, 1, 0);
void klog_printf(const char *str, ...) PBOS_FMTARG(printf, 1, 2);
void klog_putc(char ch);
void klog_puts(const char *str);

#ifndef NDEBUG
	#define dbg_vprintf klog_vprintf
	#define dbg_printf klog_printf
#else
	#define dbg_vprintf
	#define dbg_printf
#endif

#define kd_printf(component, fmt, ...) klog_printf("[%s]" fmt, component, ##__VA_ARGS__)
#define kd_println(component, fmt, ...) klog_printf("[%s]" fmt "\n", component, ##__VA_ARGS__)

PBOS_EXTERN_C_END

#endif
