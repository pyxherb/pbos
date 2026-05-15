#ifndef _PBOS_KFXX_ALLOCATOR_H_
#define _PBOS_KFXX_ALLOCATOR_H_

#include <pbos/kd/assert.h>
#include "utils.hh"

#ifdef __cplusplus
namespace kfxx {
	class Allocator {
	public:
		PBOS_API Allocator();
		PBOS_API virtual ~Allocator() = 0;

		virtual size_t inc_ref() noexcept = 0;
		virtual size_t dec_ref() noexcept = 0;

		virtual void *alloc(size_t size, size_t alignment) noexcept = 0;
		virtual void *realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept = 0;
		virtual void *realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept = 0;
		virtual void release(void *ptr, size_t size, size_t alignment) noexcept = 0;

		virtual void *type_identity() const noexcept = 0;
	};

	class KernelAllocator : public kfxx::Allocator {
	public:
		PBOS_API KernelAllocator();
		PBOS_API virtual ~KernelAllocator();

		PBOS_API virtual size_t inc_ref() noexcept override;
		PBOS_API virtual size_t dec_ref() noexcept override;

		PBOS_API virtual void *alloc(size_t size, size_t alignment) noexcept override;
		PBOS_API virtual void *realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept override;
		PBOS_API virtual void *realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept override;
		PBOS_API virtual void release(void *ptr, size_t size, size_t alignment) noexcept override;

		PBOS_API virtual void *type_identity() const noexcept override;
	};

	PBOS_API extern KernelAllocator g_kernel_allocator;

	///
	/// @brief Get the global kernel allocator.
	///
	/// @return Pointer to the global kernel allocaotr.
	///
	PBOS_FORCEINLINE KernelAllocator *kernel_allocator() noexcept {
		return &g_kernel_allocator;
	}

	PBOS_FORCEINLINE void verify_allocator(const Allocator *x, const Allocator *y) {
		if (x && y) {
			// Check if the allocators have the same type.
			kd_dbgcheck(x->type_identity() == y->type_identity(), "Incompatible allocators");
		}
	}

	template <typename T, typename... Args>
	// PBOS_REQUIRES_CONCEPT(std::constructible_from<T, Args...>)
	PBOS_FORCEINLINE T *alloc_and_construct(Allocator *alloc, Args &&...args) {
		static_assert(std::is_constructible_v<T, Args...>, "Cannot construct from provided arguments!");
		char *p = (char *)alloc->alloc(sizeof(T), alignof(T));
		if (!p)
			return nullptr;
		construct_at<T>((T *)p, std::forward<Args>(args)...);
		return (T *)p;
	}

	template <typename T>
	PBOS_FORCEINLINE void destroy_and_release(Allocator *alloc, T *const ptr) {
		kfxx::destroy_at<T>(ptr);
		alloc->release(ptr, sizeof(T), alignof(T));
	}
}
#endif

#endif
