#include <pbos/hal/irq.hh>
#include "../mm.hh"
#include <pbos/km/proc.h>

PBOS_EXTERN_C_BEGIN

mm_context_t *mm_get_cur_context() {
	return mm_cur_contexts ? mm_cur_contexts[ps_get_cur_cpuid()] : mm_kernel_context;
}

void mm_invlpg(void *ptr) {
	arch_invlpg(ptr);
}

bool mm_is_user_space(const void *ptr) {
	return ADDR_VALUE(ptr) < 0x0000800000000000ULL;
}

bool mm_probe_user_space(mm_context_t *mm_context, const void *ptr, size_t size) {
	// io::irq_disable_lock irq_lock;

	const char *p = (const char *)PGFLOOR((uintptr_t)ptr), *limit = p + PGCEIL(size);
	mm_pgaccess_t pgaccess;

	// Overflow is an error.
	if (UINTPTR_MAX - (uintptr_t)p < PGCEIL(size))
		return true;

	if((!mm_is_user_space(ptr)) || (!mm_is_user_space(limit)))
		return true;

	/*for (size_t i = 0; i < PGCEIL(size); i += PAGESIZE) {
		mm_getmap(mm_context, p + i, &pgaccess);
		if ((!(pgaccess & MM_PAGE_MAPPED)) || (!(pgaccess & MM_PAGE_USER)))
			return true;
	}*/

	return false;
}

uint8_t hn_to_kn_pmem_type(uint8_t memtype) {
	switch (memtype) {
		case MM_PHYSICAL_MEMORY_TYPE_AVAILABLE:
			return KN_PMEM_AVAILABLE;
		case MM_PHYSICAL_MEMORY_TYPE_HARDWARE:
			return KN_PMEM_HARDWARE;
		case MM_PHYSICAL_MEMORY_TYPE_HIBERNATION:
			return KN_PMEM_HIBERNATION;
		case MM_PHYSICAL_MEMORY_TYPE_ACPI:
			return KN_PMEM_ACPI;
	}
	return KN_PMEM_END;
}

PBOS_EXTERN_C_END
