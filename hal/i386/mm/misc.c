#include "../mm.h"

void kn_invlpg(void *ptr) {
	arch_invlpg(ptr);
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