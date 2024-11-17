#ifndef _ARCH_I386_KPARAM_H_
#define _ARCH_I386_KPARAM_H_

#include <pbos/attribs.h>
#include <pbos/common.h>

/// @brief Maximum number of memory maps
#define ARCH_MMAP_MAX 32

///
/// @brief Types of memory area
///
enum {
	ARCH_MEM_RESERVED = 0,
	ARCH_MEM_AVAILABLE,
	ARCH_MEM_BAD,
	ARCH_MEM_HIBERNATION,
	ARCH_MEM_ACPI,
	ARCH_MEM_END = 0xff
};

///
/// @brief Memory map entry
///
typedef struct PB_PACKED _arch_mmap_entry_t {
	uint32_t base;
	uint32_t size;
	uint8_t type;
	uint8_t reserved[7];
} arch_mmap_entry_t;

///
/// @brief APM Entry
///
typedef struct PB_PACKED _arch_apm_entry_t {
	uint8_t exist;
	uint16_t cs;
	uint16_t ds;
	uint16_t len_cs;
	uint16_t len_ds;
	void *off_entry;
	uint16_t flags;
	uint8_t reserved;
} arch_apm_entry_t;

///
/// @brief Kernel arguments structure
///
typedef struct PB_PACKED _arch_kargs_ptr_t {
	uint8_t magic[4];

	char loader_name[256];

	arch_mmap_entry_t mmaps[ARCH_MMAP_MAX];

	arch_apm_entry_t apm_entry;

	void *initcar_ptr;
	uint32_t initcar_size;
} arch_kargs_ptr_t;

/// @brief Pointer to kernel arguments
#define ARCH_KARGS_PTR ((arch_kargs_ptr_t *)0x80000)

#define KARG_MAGIC0 'K'
#define KARG_MAGIC1 'A'
#define KARG_MAGIC2 'R'
#define KARG_MAGIC3 'G'

#endif
