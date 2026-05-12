#include <pbos/kd/logger.h>
#include "../../mm.hh"

PBOS_EXTERN_C_BEGIN

static uintptr_t hn_pml4_rounddowner(uintptr_t addr) {
	return (addr & 0xffff000000000000ULL) | ((uintptr_t)UVADDR(PML4X(addr), PDPTX(addr), 0, 0, 0));
}

static uintptr_t hn_pdpt_rounddowner(uintptr_t addr) {
	return (addr & 0xffff000000000000ULL) | ((uintptr_t)UVADDR(PML4X(addr), PDPTX(addr), 0, 0, 0));
}

static uintptr_t hn_pd_rounddowner(uintptr_t addr) {
	return (addr & 0xffff000000000000ULL) | ((uintptr_t)UVADDR(PML4X(addr), PDPTX(addr), PDX(addr), 0, 0));
}

static uintptr_t hn_pt_rounddowner(uintptr_t addr) {
	return (addr & 0xffff000000000000ULL) | ((uintptr_t)UVADDR(PML4X(addr), PDPTX(addr), PDX(addr), PTX(addr), 0));
}

const ki_paging_config_t KI_PAGING_CONFIG_48BIT = {
	.pgtab_level = HN_VPM_LEVEL_MAX + 1,

	.addr_rounddowners = (hn_pglevel_addr_rounddowner_t[]){
		hn_pt_rounddowner,
		hn_pd_rounddowner,
		hn_pdpt_rounddowner,
		hn_pml4_rounddowner },
	.page_level_size = (size_t[]){ (size_t)UVADDR(0, 0, 0, 1, 0), (size_t)UVADDR(0, 0, 1, 0, 0), (size_t)UVADDR(0, 1, 0, 0, 0), (size_t)UVADDR(1, 0, 0, 0, 0) },
};

PBOS_EXTERN_C_END
