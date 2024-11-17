#ifndef KLOADER_H
#define KLOADER_H

#include <pbos/attribs.h>
#include <stdbool.h>

/// @brief Pointer to entry of the kernel image.
extern PB_NORETURN void (*boot_kentry)();

/// @brief Load the kernel image.
/// @return true if succeeded, false otherwise.
bool boot_load_pbkim();

#endif
