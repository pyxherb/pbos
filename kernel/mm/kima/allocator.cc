#include <pbos/ki/mm/kima.hh>

kima_allocator_t::kima_allocator_t() {
	kima_init_pool(&this->pool);
}

kima_allocator_t::~kima_allocator_t() {
	kima_free_pool(&this->pool);
}

size_t kima_allocator_t::inc_ref() noexcept {
	return 0;
}

size_t kima_allocator_t::dec_ref() noexcept {
	return 0;
}

void *kima_allocator_t::alloc(size_t size, size_t alignment) noexcept {
	return kima_alloc(&pool, size, alignment);
}

void *kima_allocator_t::realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	// TODO: Implement mm_krealloc and rewrite this with it.
	return kima_realloc(&pool, ptr, new_size, new_alignment);
}

void *kima_allocator_t::realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size) noexcept {
	return kima_realloc_in_place(&pool, ptr, new_size);
}

void kima_allocator_t::release(void *ptr, size_t size, size_t alignment) noexcept {
	kima_free(&pool, ptr);
}

int kima_allocator_identity;

void *kima_allocator_t::type_identity() const noexcept {
	return &kima_allocator_identity;
}
