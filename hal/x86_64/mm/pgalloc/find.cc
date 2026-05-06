#include <hal/x86_64/mm.hh>
#include "pgalloc.hh"

PBOS_EXTERN_C_BEGIN

///
/// @brief Get PMAD by a physical address.
///
/// @param addr Physical address to find.
/// @return Corresponding PMAD. NULL if not found.
///
hn_pmad_t *hn_pmad_get(void *addr) {
	PMAD_FOREACH(i) {
		if ((addr >= i->base) &&
			(addr <= ((char*)i->base + (i->len - 1))))
			return i;
	}

	return NULL;
}

void *hn_alloc_freeblk_in_area(hn_pmad_t *area) {
	if (area->free_list)
		return area->free_list->rb_value;

	return nullptr;
}

void *hn_alloc_freeblk(uint8_t type) {
	PMAD_FOREACH(i) {
		if (i->type != type)
			continue;
		void *addr = hn_alloc_freeblk_in_area(i);
		if (addr)
			return addr;
	}

	return nullptr;
}

PBOS_EXTERN_C_END
