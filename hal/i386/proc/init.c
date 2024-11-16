#include <hal/i386/proc.h>
#include <pbos/kn/km/exec.h>

om_class_t *ps_proc_class = NULL, *ps_thread_class = NULL;
ps_pcb_t *hn_proc_list = NULL;
uint32_t ps_eu_num;

void ps_init() {
	if (!(ps_proc_class = om_register_class(&PROC_CLASSID, kn_proc_destructor)))
		km_panic("Error registering process kernel class");

	if (!(ps_thread_class = om_register_class(&THREAD_CLASSID, kn_thread_destructor)))
		km_panic("Error registering thread kernel class");

	// hn_proc_list = mm_kmalloc(sizeof(ps_pcb_t) * PROC_MAX);
	// if (!hn_proc_list)
		// km_panic("Error allocating memory space for process list");

	for (size_t i = 0; i < KCTXTSWTMP_SIZE; i += PAGESIZE) {
		void *paddr = mm_pgalloc(KN_PMEM_AVAILABLE);
		if (!paddr)
			km_panic("Error allocating memory for user context area");
		if (KM_FAILED(mm_mmap(mm_kernel_context, (void *)(KCTXTSWTMP_VBASE + i), paddr, PAGESIZE, PAGE_READ | PAGE_WRITE | PAGE_USER, 0)))
			km_panic("Error mapping the user context area");
	}

	kn_init_exec();
}
