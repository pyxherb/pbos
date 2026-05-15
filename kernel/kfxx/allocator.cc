#include <pbos/kfxx/allocator.hh>
#include <pbos/mm/mm.h>

using namespace kfxx;

PBOS_API Allocator::Allocator() {
}

PBOS_API Allocator::~Allocator() {
}

PBOS_API KernelAllocator::KernelAllocator() {
}

PBOS_API KernelAllocator::~KernelAllocator() {
}

PBOS_API size_t KernelAllocator::inc_ref() noexcept {
	return 0;
}

PBOS_API size_t KernelAllocator::dec_ref() noexcept {
	return 0;
}

PBOS_API void *KernelAllocator::alloc(size_t size, size_t alignment) noexcept {
	return mm_kalloc(size, alignment);
}

PBOS_API void *KernelAllocator::realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	// TODO: Implement mm_krealloc and rewrite this with it.
	return mm_krealloc(ptr, new_size, new_alignment);
}

PBOS_API void *KernelAllocator::realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	return mm_krealloc_in_place(ptr, new_size, new_alignment);
}

PBOS_API void KernelAllocator::release(void *ptr, size_t size, size_t alignment) noexcept {
	mm_kfree(ptr);
}

int _ki_kernel_allocator_identity;

PBOS_API void *KernelAllocator::type_identity() const noexcept {
	return &_ki_kernel_allocator_identity;
}

PBOS_API KernelAllocator kfxx::g_kernel_allocator;
