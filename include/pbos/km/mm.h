#ifndef _PBOS_KM_MM_H_
#define _PBOS_KM_MM_H_

#include <pbos/common.h>
#include <pbos/generated/km.h>
#include "result.h"

enum {
	MM_PMEM_AVAILABLE = 0,
	MM_PMEM_HARDWARE,
	MM_PMEM_HIBERNATION,
	MM_PMEM_ACPI
};

#define PAGE_MAPPED 0x01   // Mapped
#define PAGE_READ 0x02	   // Read
#define PAGE_WRITE 0x04	   // Write
#define PAGE_EXEC 0x08	   // Execute
#define PAGE_NOCACHE 0x10  // Non-cached
#define PAGE_USER 0x20	   // User

typedef uint8_t mm_pgaccess_t;
typedef uint8_t mm_order_t;

typedef struct _mm_context_t mm_context_t;

mm_order_t mm_max_order();
size_t mm_getpgsize();

PB_NODISCARD void *mm_pgalloc(uint8_t memtype);
void mm_pgfree(void *ptr);

void mm_refpg(void *ptr);

PB_NODISCARD void *mm_kmalloc(size_t size);

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
PB_NODISCARD void *mm_vmalloc(
	mm_context_t *context,
	const void *minaddr,
	const void *maxaddr,
	size_t size,
	mm_pgaccess_t access,
	mm_vmalloc_flags_t flags);

/// @brief Allocate a virtual memory space in kernel area.
///
/// @param context Target memory context.
/// @param size Size for allocation.
/// @param access Page access for allocation.
/// @return Pointer to allocated virtual address, NULL if failed.
PB_NODISCARD void *mm_kvmalloc(mm_context_t *context, size_t size, mm_pgaccess_t access, mm_vmalloc_flags_t flags);

/// @brief Free a virtual memory space.
///
/// @param context Target memory context.
/// @param addr Pointer to allocated virtual address.
/// @param size Previous allocated size.
void mm_vmfree(mm_context_t *context, void *addr, size_t size);

#define MMAP_NOSETVPM 0x40000000
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

void *mm_getmap(mm_context_t *context, const void *vaddr, mm_pgaccess_t *pgaccess_out);

PB_NODISCARD km_result_t kn_mm_init_context(mm_context_t *context);
void mm_free_context(mm_context_t *context);
void mm_switch_context(mm_context_t *context);

void mm_invlpg(void *ptr);

bool mm_is_user_access_violated(mm_context_t *mm_context, const void *ptr, size_t size);

/// @brief The kernel MM context.
extern mm_context_t *mm_kernel_context;
/// @brief An array that contains current MM contexts of each EU.
extern mm_context_t **mm_cur_contexts;

#endif
