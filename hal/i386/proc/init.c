#include <hal/i386/proc.h>

om_class_t *ps_proc_class = NULL, *ps_thread_class = NULL;
ps_pcb_t *hn_proc_list = NULL;

void ps_init() {
	if (!(ps_proc_class = om_register_class(&PROC_CLASSID, kn_proc_destructor)))
		km_panic("Error registering process kernel class");

	if(!(ps_thread_class=om_register_class(&THREAD_CLASSID,kn_thread_destructor)))
		km_panic("Error registering thread kernel class");

	hn_proc_list = mm_kmalloc(sizeof(ps_pcb_t) * PROC_MAX);
	if (!hn_proc_list)
		km_panic("Error allocating memory space for process list");
}
