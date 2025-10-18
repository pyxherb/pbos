#ifndef _PBOS_KM_MM_H_
#define _PBOS_KM_MM_H_

#include <pbos/common.h>
#include <pbos/generated/km.h>
#include "result.h"

PBOS_EXTERN_C_BEGIN

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

typedef struct _mm_context_t mm_context_t;

///
/// @brief Allocate a single physical page.
///
/// @param memtype Type of the page to be allocated.
/// @return Physical address of allocated page, NULL if failed.
///
PBOS_NODISCARD void *mm_pgalloc(uint8_t memtype);
///
/// @brief Free a single physical page.
/// @details This function decrease the reference count of specified page, when
/// the reference count become 0, the page will be marked as free.
///
/// @param ptr Physical address of page to be freed.
///
void mm_pgfree(void *ptr);

///
/// @brief Increase reference counter of a physical page.
///
/// @param ptr Physical address of page to be referenced.
///
void mm_refpg(void *ptr);

///
/// @brief Allocate a memory block from the default pool.
///
/// @param size Size of the memory bloc to be allocated.
/// @return Virtual address to the memory block, NULL if failed.
///
PBOS_NODISCARD void *mm_kmalloc(size_t size);

///
/// @brief Free a memory block from the default pool.
///
/// @param ptr Virtual address to the block to be freed.
///
void mm_kfree(void *ptr);

/// @brief Do not mark the virtual pages as reserved after the allocation.
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
PBOS_NODISCARD void *mm_vmalloc(
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
PBOS_NODISCARD void *mm_kvmalloc(mm_context_t *context, size_t size, mm_pgaccess_t access, mm_vmalloc_flags_t flags);

/// @brief Free a virtual memory space.
///
/// @param context Target memory context.
/// @param addr Pointer to allocated virtual address.
/// @param size Previous allocated size.
void mm_vmfree(mm_context_t *context, void *addr, size_t size);

#define MMAP_NOSETVPM 0x40000000
#define MMAP_NORC 0x80000000
#define MMAP_IOREMAP MMAP_NORC

typedef uint32_t mmap_flags_t;

///
/// @brief Map a continuous physical memory region to a continuous virtual memory region.
///
/// @param context Memory context to be operated.
/// @param vaddr Base of virtual memory space to be mapped.
/// @param paddr Base of physical memory space to be mapped.
/// @param size Size of the region to be mapped.
/// @param access Page access of mapped virtual pages.
/// @param flags Flags for mapping.
/// @return The result code for the mapping operation.
///
km_result_t mm_mmap(
	mm_context_t *context,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_pgaccess_t access,
	mmap_flags_t flags);

///
/// @brief Set page access of virtual pages.
///
/// @param context Memory context to be operated.
/// @param vaddr Base of virtual memory space to be operated.
/// @param size Size of virtual memory space to be operated.
/// @param access Page access to be applied to the virtual pages.
///
void mm_chpgmod(
	mm_context_t *context,
	const void *vaddr,
	size_t size,
	mm_pgaccess_t access);

///
/// @brief Unmap a continuous virtual memory region.
///
/// @param context Memory context to be operated.
/// @param vaddr Base of virtual memory space to be unmapped.
/// @param size Size of virtual memory space to be unmapped.
/// @param flags Flags for unmapping, same as mmap's.
///
void mm_unmmap(mm_context_t *context, void *vaddr, size_t size, mmap_flags_t flags);

///
/// @brief Get mapping of a virtual page.
///
/// @param context Memory context to be operated.
/// @param vaddr Virtual address of the virtual page to be queried.
/// @param pgaccess_out Where to store page access of the page.
/// @return Corresponding physical address of the mapped virtual page.
///
void *mm_getmap(mm_context_t *context, const void *vaddr, mm_pgaccess_t *pgaccess_out);

///
/// @brief Initialize a memory context.
///
/// @param context Pointer to the memory context buffer.
/// @return The result code for the initialization operation.
///
PBOS_NODISCARD km_result_t kn_mm_init_context(mm_context_t *context);
///
/// @brief Free a memory context and its associated resource.
///
/// @param context Pointer to the memory context to be freed.
///
void mm_free_context(mm_context_t *context);
///
/// @brief Switch current memory context.
///
/// @param context Context to be switched to.
///
void mm_switch_context(mm_context_t *context);

///
/// @brief Invalidate mapping of a page in the TLB.
///
/// @param ptr Pointer to the virtual address to be invalidated.
///
void mm_invlpg(void *ptr);

///
/// @brief Check if accessing a space of memory in user mode violates the memory protection.
///
/// @param mm_context Memory context for checking.
/// @param ptr Address of the space to be accessed.
/// @param size Size of the space to be accessed.
/// @return true The accessing violates the memory protection.
/// @return false The accessing does not violate the memory protection.
///
bool mm_probe_user_space(mm_context_t *mm_context, const void *ptr, size_t size);

/// @brief The kernel memory context.
extern mm_context_t *mm_kernel_context;

/// @brief An array that contains current memory contexts of each EU.
extern mm_context_t **mm_cur_contexts;

PBOS_EXTERN_C_END

#endif
