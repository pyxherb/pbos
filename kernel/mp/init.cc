#include <pbos/kn/mp/init.hh>
#include <pbos/kn/mm/mm.hh>
#include <pbos/hal/irq.hh>
#include <string.h>

void mp_alloc_resources() {
	if (!(mm_cur_contexts = (mm_context_t **)mm_kmalloc(mp_num_total_cpu * sizeof(mm_context_t *), alignof(mm_context_t *)))) {
		km_panic("Unable to allocate memory context for all CPUs");
	}

	for (ps_cpu_id_t i = 0; i < mp_num_total_cpu; ++i) {
		mm_cur_contexts[i] = mm_kernel_context;
	}

	// Allocate IRQ contexts.
	if (!(irq_contexts = (hal_irq_context_t **)mm_kmalloc(mp_num_total_cpu * sizeof(hal_irq_context_t *), alignof(hal_irq_context_t *)))) {
		km_panic("Unable to allocate interrupt context for all CPUs");
	}

	memset(irq_contexts, 0, mp_num_total_cpu * sizeof(hal_irq_context_t *));
}
