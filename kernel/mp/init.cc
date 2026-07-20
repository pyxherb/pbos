#include <pbos/ki/mp/init.hh>
#include <pbos/ki/mm/context.hh>
#include <pbos/ki/io/context.hh>
#include <pbos/kf/misc.h>

void ki_mp_alloc_resources() {
	if (!(mm_cur_contexts = (mm_context_t **)mm_kalloc(mp_num_total_cpu * sizeof(mm_context_t *), alignof(mm_context_t *)))) {
		km_panic("Unable to allocate memory context array for all CPUs");
	}

	for (ps_cpuid_t i = 0; i < mp_num_total_cpu; ++i) {
		mm_cur_contexts[i] = mm_kernel_context;
	}

	if(!(ki_per_cpu_io_contexts = (io_context_t **)mm_kalloc(mp_num_total_cpu * sizeof(io_context_t *), alignof(io_context_t *)))){
		km_panic("Unable to allocate IO context array for all CPUs");
	}

	memset(ki_per_cpu_io_contexts, 0, mp_num_total_cpu * sizeof(io_context_t *));

	for(size_t i = 0 ; i < mp_num_total_cpu; ++i) {
		io_context_t *context = kfxx::alloc_and_construct<io_context_t>(kfxx::kernel_allocator());
		if(!context)
			km_panic("Unable to allocate IO context for CPU %zu", i);
		ki_per_cpu_io_contexts[i] = context;
	}
}
