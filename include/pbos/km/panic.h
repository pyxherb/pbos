#ifndef _PBOS_KM_PANIC_H_
#define _PBOS_KM_PANIC_H_

#include <pbos/common.h>

/// @brief Check if the system was panicked.
///
/// @return true The system was panicked.
/// @return false The system did not panic.
bool km_ispanicked();

/// @brief Panic and shutdown the system.
///
/// @param fmt Formatted panic message to display.
/// @param ... Arguments for the formatted message.
PB_NORETURN void km_panic(const char* fmt, ...);

#endif
