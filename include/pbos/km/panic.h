#ifndef _PBOS_KM_PANIC_H_
#define _PBOS_KM_PANIC_H_

#include <pbos/common.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Check if the system is panicked.
///
/// @return true The system is panicked.
/// @return false The system did is not panicked.
bool km_is_panicked();

/// @brief Panic and shutdown the system.
///
/// @param fmt Formatted panic message to display.
/// @param ... Arguments for the formatted message.
PB_NORETURN void km_panic(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
