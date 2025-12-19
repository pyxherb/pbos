#ifndef _PBOS_KFXX_ALLOCATOR_H_
#define _PBOS_KFXX_ALLOCATOR_H_

#include <pbos/km/assert.h>
#include "basedefs.hh"

#ifdef __cplusplus
namespace kfxx {
	class allocator {
	public:
		PBOS_KFXX_API allocator();
		PBOS_KFXX_API virtual ~allocator() = 0;

		virtual size_t inc_ref() noexcept = 0;
		virtual size_t dec_ref() noexcept = 0;

		virtual void *alloc(size_t size, size_t alignment) noexcept = 0;
		virtual void *realloc(void *ptr, size_t size, size_t alignment, size_t newSize, size_t newAlignment) noexcept = 0;
		virtual void release(void *ptr, size_t size, size_t alignment) noexcept = 0;

		virtual void *type_identity() const noexcept = 0;
	};

	PBOS_FORCEINLINE void verify_allocator(const allocator *x, const allocator *y) {
		if (x && y) {
			// Check if the allocators have the same type.
			kd_assert(("Incompatible allocators", x->type_identity() == y->type_identity()));
		}
	}
}
#endif

#endif
