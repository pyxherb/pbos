#ifndef _PBOS_KM_MM_H_
#define _PBOS_KM_MM_H_

#include <pbos/common.h>
#include <pbos/generated/km.h>
#include <pbos/km/result.h>
#ifndef __cplusplus
	#include <stdalign.h>
#endif

PBOS_EXTERN_C_BEGIN

enum {
	/// @brief Physical memory type that is available for common storage purposes.
	MM_PHYSICAL_MEMORY_TYPE_AVAILABLE = 0,
	/// @brief Physical memory type that is reserved for hardwares.
	MM_PHYSICAL_MEMORY_TYPE_HARDWARE,
	/// @brief Physical memory type that is critical to the system and thus to be reserved.
	MM_PHYSICAL_MEMORY_TYPE_CRITICAL,
	/// @brief Physical memory type that contains data used during system startup.
	MM_PHYSICAL_MEMORY_TYPE_BOOTDATA,
	/// @brief Physical memory type for defective memory regions.
	MM_PHYSICAL_MEMORY_TYPE_BAD,
	/// @brief Physical memory type for reserved hibernation memory.
	MM_PHYSICAL_MEMORY_TYPE_HIBERNATION,
	/// @brief Physical memory type for ACPI reserved memory.
	MM_PHYSICAL_MEMORY_TYPE_ACPI
};

/// @brief The page is mapped.
#define MM_PAGE_MAPPED 0x01
/// @brief The page can be read.
#define MM_PAGE_READ 0x02
/// @brief The page can be written.
#define MM_PAGE_WRITE 0x04
/// @brief The page can be executed.
#define MM_PAGE_EXEC 0x08
/// @brief The page should not be cached.
#define MM_PAGE_NOCACHE 0x10
/// @brief The page can be accessed by the user.
#define MM_PAGE_USER 0x20

/// @brief Type used for representing access mode of a page.
typedef uint32_t mm_pgaccess_t;

typedef struct _mm_vmr_t mm_vmr_t;
typedef struct _mm_context_t mm_context_t;

///
/// @brief Allocate a single physical page.
/// @note To release the allocated page, call @c mm_unpin_page.
///
/// @param memtype Type of the page to be allocated.
/// @return Physical address of allocated page, NULL if failed.
///
PBOS_NODISCARD void *mm_pgalloc(uint8_t memtype);

///
/// @brief Unreference a single physical page.
/// @details This function decrease the reference count of specified page, when
/// the reference count become 0, the page will be marked as free.
///
/// @param ptr Physical address of page to be freed.
///
void mm_unref_page(void *ptr);

typedef uint32_t mm_iommap_flags_t;

PBOS_NODISCARD km_result_t mm_iommap(
	mm_context_t *context,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_pgaccess_t access,
	mm_iommap_flags_t flags);

void mm_uniommap(mm_context_t *context, void *vaddr, size_t size, mm_iommap_flags_t flags);

///
/// @brief Increase reference counter of a physical page. This does not prevent the page from swapping out.
///
/// @param ptr Physical address of page to be referenced.
///
void mm_ref_page(void *ptr);

///
/// @brief Pin a page to prevent it from being swapped out.
///
/// @param ptr Physical address of the page.
///
void mm_pin_page(void *ptr);

///
/// @brief Unpin a page.
///
/// @param ptr Physical address of the page.
///
void mm_unpin_page(void *ptr);

///
/// @brief Allocate a memory block from the default pool.
///
/// @param size Size of the memory bloc to be allocated.
/// @return Virtual address to the memory block, NULL if failed.
///
PBOS_NODISCARD PBOS_API void *mm_kalloc(size_t size, size_t alignment);

///
/// @brief Reallocated an allocated memory block.
///
/// @param old_ptr Pointer to the old block.
/// @param size Wanted size of the new block.
/// @param alignment Alignment of the new block./
/// @return Pointer to newly allocated block, or @c NULL will be returned if failed (the old block will be kept).
///
PBOS_NODISCARD PBOS_API void *mm_krealloc(void *old_ptr, size_t size, size_t alignment);

///
/// @brief Reallocated an allocated memory block in place (does not change the address).
///
/// @param old_ptr Pointer to the old block.
/// @param size Wanted size of the new block.
/// @param alignment Alignment of the new block./
/// @return Pointer to the resized block, or @c NULL will be returned if failed (the content will be kept).
///
PBOS_NODISCARD PBOS_API void *mm_krealloc_in_place(void *old_ptr, size_t size, size_t alignment);

///
/// @brief Free a memory block from the default pool.
///
/// @param ptr Virtual address to the block to be freed.
///
void PBOS_API mm_kfree(void *ptr);

typedef struct _ps_pcb_t ps_pcb_t;

PBOS_NODISCARD void *mm_ppalloc(ps_pcb_t *proc, size_t size, size_t alignment);

PBOS_NODISCARD void *mm_pprealloc(ps_pcb_t *proc, void *old_ptr, size_t size, size_t alignment);

PBOS_NODISCARD void *mm_pprealloc_in_place(ps_pcb_t *proc, void *old_ptr, size_t size, size_t alignment);

void mm_ppfree(ps_pcb_t *proc, void *ptr);

/// @brief Do not mark the virtual pages as reserved after the allocation.
#define VMALLOC_ATOMIC 0x00000001
#define VMALLOC_NOSETVPM 0x40000000
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

#define MMAP_ATOMIC 0x00000001
#define MMAP_NO_REMAP 0x00000002
/// @brief Do not increase the reference count, usually used for mapping page allocated by @c mm_pgalloc
#define MMAP_NO_INC_RC 0x00000004
#define MMAP_IGNORE_VMR 0x40000000
#define MMAP_NO_PGTAB_ALLOC 0x40000000

typedef uint32_t mmap_flags_t;

///
/// @brief Map a continuous physical memory region to a continuous virtual memory region.
/// @note The mapped pages will be referenced (if is user) or pinned (if is kernel). Use @c MMAP_NO_INC_RC to avoid this.
///
/// @param context Memory context to be operated.
/// @param vaddr Base of virtual memory space to be mapped.
/// @param paddr Base of physical memory space to be mapped.
/// @param size Size of the region to be mapped.
/// @param access Page access of mapped virtual pages.
/// @param flags Flags for mapping.
/// @return The result code for the mapping operation.
///
PBOS_NODISCARD PBOS_API km_result_t mm_mmap(
	mm_context_t *context,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_pgaccess_t access,
	mmap_flags_t flags);

///
/// @brief Map a series of physical pages to
///
/// @param context
/// @param vaddr
/// @param paddrs
/// @param size
/// @param access
/// @param flags
/// @return PBOS_NODISCARD
///
PBOS_NODISCARD PBOS_API km_result_t mm_merge_mapped_area(
	mm_context_t *context,
	void *vaddr_a,
	void *vaddr_b);

PBOS_NODISCARD PBOS_API km_result_t mm_split_mapped_area(
	mm_context_t *context,
	void *area_vaddr,
	void *split_point);

///
/// @brief Set page access of virtual pages.
///
/// @param context Memory context to be operated.
/// @param vaddr Base of virtual memory space to be operated.
/// @param size Size of virtual memory space to be operated.
/// @param access Page access to be applied to the virtual pages.
///
PBOS_NODISCARD PBOS_API km_result_t mm_set_page_access(
	mm_context_t *context,
	void *vaddr,
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
PBOS_API km_result_t mm_unmmap(mm_context_t *context, void *vaddr, size_t size, mmap_flags_t flags);

///
/// @brief Get mapping of a virtual page.
///
/// @param context Memory context to be operated.
/// @param vaddr Virtual address of the virtual page to be queried.
/// @param pgaccess_out Where to store page access of the page.
/// @return Corresponding physical address of the mapped virtual page.
///
PBOS_API void *mm_getmap(mm_context_t *context, const void *vaddr, mm_pgaccess_t *pgaccess_out);

PBOS_NODISCARD km_result_t mm_mmap(
	mm_context_t *context,
	void *vaddr,
	void *paddr,
	size_t size,
	mm_pgaccess_t access,
	mmap_flags_t flags);

///
/// @brief Initialize a memory context.
///
/// @param context Pointer to the memory context buffer.
/// @return The result code for the initialization operation.
///
PBOS_NODISCARD PBOS_API km_result_t mm_alloc_context(mm_context_t *cur_context, mm_context_t **new_context_out);
///
/// @brief Free a memory context and its associated resource.
///
/// @param context Pointer to the memory context to be freed.
///
PBOS_API void mm_free_context(mm_context_t *context);
///
/// @brief Switch current memory context.
///
/// @param context Context to be switched to.
///
PBOS_API void mm_switch_context(mm_context_t *context);

///
/// @brief Invalidate mapping of a page in the TLB.
///
/// @param ptr Pointer to the virtual address to be invalidated.
///
PBOS_API void mm_invl_page(void *ptr);

PBOS_PURE PBOS_API bool mm_is_user_space(const void *ptr);

PBOS_API mm_vmr_t *mm_lookup_area(mm_context_t *mm_context, void *ptr);

PBOS_API km_result_t mm_probe_kernel_pages(mm_context_t *mm_context, void *ptr, size_t size, mm_pgaccess_t access);

PBOS_API km_result_t mm_probe_and_lock_user_pages(mm_context_t *mm_context, void *ptr, size_t size, mm_pgaccess_t access);

PBOS_API km_result_t mm_unlock_pages(mm_context_t *mm_context, void *ptr, size_t size);

/// @brief The kernel memory context.
extern mm_context_t *mm_kernel_context;

PBOS_PURE PBOS_API mm_context_t *mm_get_cur_context();

PBOS_PURE PBOS_API size_t mm_get_page_size();

PBOS_EXTERN_C_END

#endif
