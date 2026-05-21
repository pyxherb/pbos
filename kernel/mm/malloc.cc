#include <pbos/ki/mm/kima.hh>
#include <pbos/kd/logger.h>
#include <pbos/ki/km/symbol.hh>

kima_pool_t ki_global_pool_storage;
kima_pool_t *mm_global_pool = &ki_global_pool_storage;

void ki_mm_init_global_allocator() {
	ki_init_kima_pool(&ki_global_pool_storage);
}

PBOS_NODISCARD PBOS_API void *mm_kalloc(size_t size, size_t alignment) {
	return kima_alloc(mm_global_pool, size, alignment);
}

PBOS_NODISCARD PBOS_API void *mm_krealloc(void *old_ptr, size_t size, size_t alignment) {
	return kima_realloc(mm_global_pool, old_ptr, size, alignment);
}

PBOS_NODISCARD PBOS_API void *mm_krealloc_in_place(void *old_ptr, size_t size, size_t alignment) {
	return kima_realloc_in_place(mm_global_pool, old_ptr, size, alignment);
}

PBOS_API void mm_kfree(void *ptr) {
	kima_free(mm_global_pool, ptr);
}
