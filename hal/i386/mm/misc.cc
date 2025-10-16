#include "../mm.h"

PBOS_EXTERN_C_BEGIN

void mm_invlpg(void *ptr) {
	arch_invlpg(ptr);
}

bool mm_probe_user_space(mm_context_t *mm_context, const void *ptr, size_t size) {
	const char *p = (const char *)PGFLOOR((uintptr_t)ptr), *limit = p + PGCEIL(size);
	mm_pgaccess_t pgaccess;

	// Overflow is an error.
	if (UINTPTR_MAX - (uintptr_t)p < PGCEIL(size))
		return true;

	if ((uintptr_t)p < (uintptr_t)USPACE_VBASE)
		return true;

	if ((p >= (char *)KERNEL_VBASE) || (limit >= (char *)KERNEL_VBASE))
		return true;

	for (size_t i = 0; i < PGCEIL(size); ++i) {
		mm_getmap(mm_context, p, &pgaccess);
		if ((!(pgaccess & PAGE_MAPPED)) || (!(pgaccess & PAGE_USER)))
			return true;
	}

	return false;
}

uint8_t hn_to_kn_pmem_type(uint8_t memtype) {
	switch (memtype) {
		case MM_PMEM_AVAILABLE:
			return KN_PMEM_AVAILABLE;
		case MM_PMEM_HARDWARE:
			return KN_PMEM_HARDWARE;
		case MM_PMEM_HIBERNATION:
			return KN_PMEM_HIBERNATION;
		case MM_PMEM_ACPI:
			return KN_PMEM_ACPI;
	}
	return KN_PMEM_END;
}

PBOS_EXTERN_C_END
