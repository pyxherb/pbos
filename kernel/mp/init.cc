#include <pbos/ki/mp/init.hh>
#include <pbos/ki/mm/context.hh>
#include <string.h>

void ki_mp_alloc_resources() {
	if (!(mm_cur_contexts = (mm_context_t **)mm_kalloc(mp_num_total_cpu * sizeof(mm_context_t *), alignof(mm_context_t *)))) {
		km_panic("Unable to allocate memory context for all CPUs");
	}

	for (ps_cpuid_t i = 0; i < mp_num_total_cpu; ++i) {
		mm_cur_contexts[i] = mm_kernel_context;
	}
}
