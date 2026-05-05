#ifndef _PBOS_HAL_X86_64_MISC_H_
#define _PBOS_HAL_X86_64_MISC_H_

#include <pbos/common.h>
#include <limine.h>

PBOS_EXTERN_C_BEGIN

#define HN_REQUIRED_LIMINE_REVISION LIMINE_BASE_REVISION(4)

extern volatile uint64_t hn_limine_base_revision[];
extern volatile uint64_t hn_limine_requests_start_marker[];
extern volatile struct limine_framebuffer_request hn_limine_framebuffer_request;
extern volatile struct limine_hhdm_request hn_limine_hhdm_request;
extern volatile struct limine_executable_address_request hn_limine_executable_address_request;
extern volatile struct limine_memmap_request hn_limine_memmap_request;
extern volatile struct limine_rsdp_request hn_limine_rsdp_request;
extern volatile struct limine_module_request hn_limine_module_request;
extern volatile uint64_t hn_limine_requests_end_marker[];

typedef void (*hn_ctor_t)();
typedef void (*hn_dtor_t)();

// extern hn_ctor_t *KN_CTORS_BEGIN, *KN_CTORS_END;
// extern hn_dtor_t *KN_DTORS_BEGIN, *KN_DTORS_END;

PBOS_EXTERN_C_END

#endif
