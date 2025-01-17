#include <hal/i386/proc.h>
#include <pbos/kn/km/exec.h>

om_class_t *ps_proc_class = NULL, *ps_thread_class = NULL;
kf_rbtree_t ps_global_proc_set;
uint32_t ps_eu_num;

static bool _ps_pcb_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y) {
	return PB_CONTAINER_OF(ps_pcb_t, node_header, x)->proc_id < PB_CONTAINER_OF(ps_pcb_t, node_header, y)->proc_id;
}

static bool _ps_pcb_nodefree() {
	// stub
}

void ps_init() {
	if (!(ps_proc_class = om_register_class(&PROC_CLASSID, kn_proc_destructor)))
		km_panic("Error registering process kernel class");

	if (!(ps_thread_class = om_register_class(&THREAD_CLASSID, kn_thread_destructor)))
		km_panic("Error registering thread kernel class");

	kf_rbtree_init(
		&ps_global_proc_set,
		_ps_pcb_nodecmp,
		_ps_pcb_nodefree);

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

	if (!(ps_cur_procs = mm_kmalloc(ps_eu_num * sizeof(ps_pcb_t *)))) {
		km_panic("Unable to allocate current PCB pointer storage for all CPUs");
	}

	memset(ps_cur_procs, 0, ps_eu_num * sizeof(ps_pcb_t *));

	if (!(ps_cur_threads = mm_kmalloc(ps_eu_num * sizeof(ps_tcb_t *)))) {
		km_panic("Unable to allocate current TCB pointer storage for all CPUs");
	}

	memset(ps_cur_threads, 0, ps_eu_num * sizeof(ps_tcb_t *));

	kn_init_exec();
}
