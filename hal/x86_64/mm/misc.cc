#include <pbos/ps/proc.h>
#include <pbos/hal/irq.hh>
#include "../mm.hh"

PBOS_EXTERN_C_BEGIN

mm_context_t *mm_get_cur_context() {
	return mm_cur_contexts ? mm_cur_contexts[ps_get_cur_cpuid()] : mm_kernel_context;
}

void mm_invl_page(void *ptr) {
	arch_invlpg(ptr);
}

bool mm_is_user_space(const void *ptr) {
	return ptr < (void *)0x0000800000000000ULL;
}

PBOS_EXTERN_C_END
