#include <pbos/kfxx/allocator.hh>
#include <pbos/mm/mm.h>

using namespace kfxx;

PBOS_KFXX_API allocator_t::allocator_t() {
}

PBOS_KFXX_API allocator_t::~allocator_t() {
}

PBOS_API kernel_allocator_t::kernel_allocator_t() {
}

PBOS_API kernel_allocator_t::~kernel_allocator_t() {
}

PBOS_API size_t kernel_allocator_t::inc_ref() noexcept {
	return 0;
}

PBOS_API size_t kernel_allocator_t::dec_ref() noexcept {
	return 0;
}

PBOS_API void *kernel_allocator_t::alloc(size_t size, size_t alignment) noexcept {
	kd_assert(alignment >= alignof(max_align_t));
	return mm_kmalloc(size, alignof(max_align_t));
}

PBOS_API void *kernel_allocator_t::realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	// TODO: Implement mm_krealloc and rewrite this with it.
	kd_assert(alignment >= alignof(max_align_t));
	return mm_krealloc(ptr, new_size, new_alignment);
}

PBOS_API void *kernel_allocator_t::realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	// TODO: Implement mm_krealloc and rewrite this with it.
	kd_assert(alignment >= alignof(max_align_t));
	// return mm_krealloc(ptr, new_size, new_alignment);
	return nullptr;
}

PBOS_API void kernel_allocator_t::release(void *ptr, size_t size, size_t alignment) noexcept {
	mm_kfree(ptr);
}

int kn_fs_fcb_allocator_identity;

PBOS_API void *kernel_allocator_t::type_identity() const noexcept {
	return &kn_fs_fcb_allocator_identity;
}

extern kernel_allocator_t kfxx::g_kernel_allocator;
