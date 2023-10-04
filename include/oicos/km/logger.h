#ifndef _OICOS_KM_LOGGER_H_
#define _OICOS_KM_LOGGER_H_

#include <oicos/attribs.h>
#include <oicos/common.h>
#include <stdarg.h>

#define KLOG_COLOR_BLACK 0x00
#define KLOG_COLOR_BLUE 0x01
#define KLOG_COLOR_GREEN 0x02
#define KLOG_COLOR_CYAN 0x03
#define KLOG_COLOR_RED 0x04
#define KLOG_COLOR_PURPLE 0x05
#define KLOG_COLOR_YELLOW 0x06
#define KLOG_COLOR_WHITE 0x07
#define KLOG_COLOR_LIGHT 0x08

enum {
	KLOG_CAP_COLOR = 0
};

enum {
	KLOG_LEVEL_DEBUG = 0,  // Debug information
	KLOG_LEVEL_INFO,	   // Information
	KLOG_LEVEL_NOTE,	   // Information that should be noticed by users
	KLOG_LEVEL_WARNING,	   // Warnings
	KLOG_LEVEL_ERROR,	   // Errors that do not cause the kernel to become unstable
	KLOG_LEVEL_FATAL,	   // Errors that may cause the kernel to become unstable
};

enum {
	KLOG_MODE_INIT = 0,	 // Initialize the logger.
	KLOG_MODE_GETCAP,	 // Check if the logger has specified capability.
	KLOG_MODE_PRINT,	 // Output plain text.
	KLOG_MODE_PRINTFMT,	 // Output formatted text.
	KLOG_MODE_EXCALL	 // Call an extended function.
};

typedef size_t (*klog_logger_t)(uint16_t mode, const void *arg, va_list vargs);

/// @brief Set current logger for kernel logger system.
///
/// @param logger Logger callback (backend) to set.
void klog_setlogger(klog_logger_t logger);

/// @brief Get default kernel logger.
///
/// @return The default kernel logger.
klog_logger_t klog_getdefault();

/// @brief Get current kernel logger.
///
/// @return Current kernel logger.
klog_logger_t klog_getlogger();

/// @brief Check if current kernel logger is capable of specified capability.
///
/// @param cap Capability to check.
/// @return true The logger is capable of the capability.
/// @return false The logger is not capable of the capability.
bool klog_iscapable(uint16_t cap);

#define KLOG_EXCALL_COLOR 0x00000000

#define KLOG_EXCALL_PRIVATE_MIN 0xc0000000
#define KLOG_EXCALL_PRIVATE_MAX 0xffffffff

__packed_begin

	typedef struct __packed _klog_color_exarg_t {
	uint8_t fg;
	uint8_t bg;
} klog_color_exarg_t;

__packed_end

	/// @brief Call an extra function with current kernel logger.
	///
	/// @param id Extra function ID.
	/// @param ... Arguments for the extra function.
	void
	klog_excall(uint32_t id, ...);

void kvprintf(const char *str, va_list args) __format(printf, 1, 0);
void kprintf(const char *str, ...) __format(printf, 1, 2);
void kputc(char ch);
void kputs(const char *str);

#ifndef _NDEBUG
	#define kdvprintf(str, args) kvprintf(str, args)
	#define kdprintf(str, ...) kprintf(str, ##__VA_ARGS__)
#else
	#define kdvprintf(str, args)
	#define kdprintf(str, ...)
#endif

#endif
