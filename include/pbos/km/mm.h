#ifndef _KM_MM_H_
#define _KM_MM_H_

#include <pbos/common.h>
#include <pbos/generated/km.h>
#include "result.h"

enum {
	MM_PMEM_AVAILABLE = 0,
	MM_PMEM_HARDWARE,
	MM_PMEM_HIBERNATION,
	MM_PMEM_ACPI
};

#define PAGE_READ 0x01	   // Read
#define PAGE_WRITE 0x02	   // Write
#define PAGE_EXEC 0x04	   // Execute
#define PAGE_NOCACHE 0x08  // Non-cached
#define PAGE_USER 0x10	   // User

typedef uint8_t mm_pgaccess_t;
typedef uint8_t mm_order_t;

typedef struct _mm_context_t mm_context_t;

mm_order_t mm_max_order();
size_t mm_getpgsize();

__nodiscard void *__malloc mm_pgalloc(uint8_t memtype);
void mm_pgfree(void *ptr);

void mm_refpg(void *ptr);

__nodiscard void *__malloc mm_kmalloc(size_t size)
	__allocsize(1);

void mm_kfree(void *ptr);

#define VMALLOC_NORESERVE 0x80000000

typedef uint32_t mm_vmalloc_flags_t;

/// @brief Allocate a virtual memory space, allocated spaces will be reserved
/// until it was unmapped.
///
/// @param context Target memory context.
/// @param minaddr Minimum address for allocation.
/// @param maxaddr Maximum address for allocation.
/// @param size Size for allocation.
/// @param access Page access for allocation.
/// @return Pointer to allocated virtual address, NULL if failed.
__nodiscard void *__malloc mm_vmalloc(
	mm_context_t *context,
	const void *minaddr,
	const void *maxaddr,
	size_t size,
	mm_pgaccess_t access,
	mm_vmalloc_flags_t flags)
	__allocsize(4);

/// @brief Allocate a virtual memory space in kernel area.
///
/// @param context Target memory context.
/// @param size Size for allocation.
/// @param access Page access for allocation.
/// @return Pointer to allocated virtual address, NULL if failed.
__nodiscard void *__malloc mm_kvmalloc(mm_context_t *context, size_t size, mm_pgaccess_t access, mm_vmalloc_flags_t flags)
	__allocsize(2);

/// @brief Free a virtual memory space.
///
/// @param context Target memory context.
/// @param addr Pointer to allocated virtual address.
/// @param size Previous allocated size.
void mm_vmfree(mm_context_t *context, void *addr, size_t size);

#define MMAP_NORC 0x80000000

typedef uint32_t mmap_flags_t;

km_result_t mm_mmap(
	mm_context_t *context,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_pgaccess_t access,
	mmap_flags_t flags);

void mm_chpgmod(
	mm_context_t *context,
	const void *vaddr,
	size_t size,
	mm_pgaccess_t access);

void mm_unmmap(mm_context_t *context, void *vaddr, size_t size, mmap_flags_t flags);

void *mm_getmap(mm_context_t *context, const void *vaddr);

__nodiscard km_result_t mm_create_context(mm_context_t *context);
void mm_free_context(mm_context_t *context);
void mm_switch_context(mm_context_t *context);

extern mm_context_t *mm_kernel_context;
extern mm_context_t **mm_current_contexts;

#endif
