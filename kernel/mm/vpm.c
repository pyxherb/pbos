#include <pbos/kn/km/mm.h>

void *kn_rounddown_to_page_leveled_addr(const void *const addr, int level) {
	if(level > kn_mm_vpm_level_max)
		km_panic("Invalid page rounddown level: %d", level);
	return (void *)kn_mm_vpm_rounddowners[level]((uintptr_t)addr);
}
