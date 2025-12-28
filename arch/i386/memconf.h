#ifndef _ARCH_I386_MEMCONF_H_
#define _ARCH_I386_MEMCONF_H_

#include <pbos/kn/km/memconf.h>

PBOS_EXTERN_C_BEGIN

#define PAGESIZE 4096

#define PGADDR_MAX 0xfffff
#define PGADDR_MIN 0

/// @brief Round up a linear address into page-aligned.
#define PGCEIL(addr) ((uintptr_t)((((size_t)(addr)) + (PAGESIZE - 1)) & (~(PAGESIZE - 1))))
/// @brief Round down a linear address into page-aligned.
#define PGFLOOR(addr) ((uintptr_t)(((size_t)(addr)) & (~(PAGESIZE - 1))))

#define PAGING_LEVEL_PGDIR 0
#define PAGING_LEVEL_PGTAB 1

extern const kn_paging_config_t KN_PAGING_CONFIG_32BIT;

PBOS_EXTERN_C_END

#endif
