#ifndef _HAL_X86_64_MM_VM_HH_
#define _HAL_X86_64_MM_VM_HH_

#include <hal/x86_64/mm.hh>
#include <arch/x86_64/paging.h>
#include <pbos/km/result.h>

PBOS_EXTERN_C_BEGIN

PBOS_NODISCARD void *hali_tmpmap_early(void *paddr, size_t size, uint16_t mask);
void hali_tmpunmap_early(void *vaddr, size_t size);

PBOS_NODISCARD void *hali_tmpmap_post(void *paddr, size_t size, uint16_t mask);

void hali_tmpunmap_post(void *vaddr, size_t size);

void *hali_get_pgtab_paddr(mm_context_t *ctxt, const void *vaddr, mm_page_access_t *page_access_out);

PBOS_EXTERN_C_END

#endif
