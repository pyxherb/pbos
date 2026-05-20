#include <pbos/kfxx/allocator.hh>
#include <pbos/mm/mm.h>

using namespace kfxx;

PBOS_KERNEL_PUBLIC Alloc::Alloc() {
}

PBOS_KERNEL_PUBLIC Alloc::~Alloc() {
}

PBOS_KERNEL_PUBLIC KernelAlloc::KernelAlloc() {
}

PBOS_KERNEL_PUBLIC KernelAlloc::~KernelAlloc() {
}

PBOS_KERNEL_PUBLIC size_t KernelAlloc::inc_ref() noexcept {
	return 0;
}

PBOS_KERNEL_PUBLIC size_t KernelAlloc::dec_ref() noexcept {
	return 0;
}

PBOS_KERNEL_PUBLIC void *KernelAlloc::alloc(size_t size, size_t alignment) noexcept {
	return mm_kalloc(size, alignment);
}

PBOS_KERNEL_PUBLIC void *KernelAlloc::realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	// TODO: Implement mm_krealloc and rewrite this with it.
	return mm_krealloc(ptr, new_size, new_alignment);
}

PBOS_KERNEL_PUBLIC void *KernelAlloc::realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	return mm_krealloc_in_place(ptr, new_size, new_alignment);
}

PBOS_KERNEL_PUBLIC void KernelAlloc::release(void *ptr, size_t size, size_t alignment) noexcept {
	mm_kfree(ptr);
}

int _ki_kernel_allocator_identity;

PBOS_KERNEL_PUBLIC void *KernelAlloc::type_identity() const noexcept {
	return &_ki_kernel_allocator_identity;
}

PBOS_KERNEL_PUBLIC KernelAlloc kfxx::g_kernel_allocator;
