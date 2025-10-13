#ifndef _PBOS_KM_PANIC_H_
#define _PBOS_KM_PANIC_H_

#include <pbos/common.h>

PB_EXTERN_C_BEGIN

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

PB_EXTERN_C_END

#endif
