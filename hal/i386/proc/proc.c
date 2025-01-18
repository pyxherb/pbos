#include <hal/i386/proc.h>
#include <pbos/km/logger.h>

ps_pcb_t **ps_cur_procs;
ps_tcb_t **ps_cur_threads;

static bool _parp_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y);
static void _parp_nodefree(kf_rbtree_node_t *p);

static bool _uhr_uhandle_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y);
static void _uhr_uhandle_nodefree(kf_rbtree_node_t *p);

void kn_proc_destructor(om_object_t *obj) {
	hn_proc_cleanup((ps_pcb_t *)obj);
}

ps_pcb_t *kn_alloc_pcb() {
	ps_pcb_t *proc = mm_kmalloc(sizeof(ps_pcb_t));

	if (!proc)
		return NULL;

	memset(proc, 0, sizeof(ps_pcb_t));

	if(!(proc->mm_context = mm_kmalloc(sizeof(mm_context_t)))) {
		mm_kfree(proc);
		return NULL;
	}

	if (KM_FAILED(kn_mm_init_context(proc->mm_context))) {
		mm_kfree(proc->mm_context);
		mm_kfree(proc);
		return NULL;
	}

	om_init_object(&(proc->object_header), ps_proc_class);

	kf_rbtree_init(
		&(proc->parp_list),
		_parp_nodecmp,
		_parp_nodefree);

	kf_rbtree_init(
		&(proc->uhandle_map),
		_uhr_uhandle_nodecmp,
		_uhr_uhandle_nodefree);

	proc->last_thread_id = 0;

	proc->flags = PROC_P;

	return proc;
}

ps_pcb_t *ps_getpcb(proc_id_t pid) {
	ps_pcb_t query_node = {
		.proc_id = pid
	};
	kf_rbtree_node_t *node = kf_rbtree_find(&ps_global_proc_set, &query_node);
	if (!node)
		return NULL;
	return PB_CONTAINER_OF(ps_pcb_t, node_header, node);
}

void hn_proc_cleanup(ps_pcb_t *proc) {
	mm_free_context(proc->mm_context);
	mm_kfree(proc->mm_context);
	proc->flags &= ~PROC_A;

	kf_rbtree_foreach(i, &proc->thread_set) {
		om_decref(&(PB_CONTAINER_OF(ps_tcb_t, node_header, i)->object_header));
	}

	kf_rbtree_free(&(proc->parp_list));
}

void ps_add_thread(ps_pcb_t *proc, ps_tcb_t *thread) {
	// stub: do some checks with the new thread id, such as checking if a thread with the id exists.
	thread->thread_id = ++proc->last_thread_id;
	kf_rbtree_insert(&proc->thread_set, &thread->node_header);
}

void kn_switch_to_user_process(ps_pcb_t *pcb) {
	mm_switch_context(pcb->mm_context);
}

void kn_switch_to_user_thread(ps_tcb_t *tcb) {
	ps_load_user_context(tcb->context);
}

ps_euid_t ps_get_cur_euid() {
	return arch_storefs();
}

void kn_set_cur_euid(ps_euid_t euid) {
	arch_loadfs(euid);
}

km_result_t ps_create_uhandle(ps_pcb_t *proc, om_handle_t khandle, ps_uhandle_t *uhandle_out) {
	km_result_t result;
	ps_uhr_t *uhr = mm_kmalloc(sizeof(ps_uhr_t));

	if (!uhr) {
		return KM_RESULT_NO_MEM;
	}

	memset(uhr, 0, sizeof(ps_uhr_t));

	om_ref_handle(khandle);
	uhr->khandle = khandle;

	ps_uhandle_t initial_uhandle = proc->last_allocated_uhandle_value;
	ps_uhandle_t uhandle;

	ps_uhr_t query_uhr;
	for (;;) {
		uhandle = ++proc->last_allocated_uhandle_value;

		query_uhr.uhandle = uhandle;

		if (!kf_rbtree_find(&proc->uhandle_map, &query_uhr.node_header)) {
			break;
		}

		if (uhandle == initial_uhandle) {
			mm_kfree(uhr);
			return KM_RESULT_NO_SLOT;
		}
	}

	result = kf_rbtree_insert(&proc->uhandle_map, &uhr->node_header);

	assert(KM_SUCCEEDED(result));

	*uhandle_out = uhandle;

	return KM_RESULT_OK;
}

void ps_close_uhandle(ps_pcb_t *proc, ps_uhandle_t uhandle) {
	ps_uhr_t query_uhr;
	query_uhr.uhandle = uhandle;

	kf_rbtree_node_t *node = kf_rbtree_find(&proc->uhandle_map, &query_uhr.node_header);
	assert(node);

	ps_uhr_t *uhr = PB_CONTAINER_OF(ps_uhr_t, node_header, node);

	om_close_handle(uhr->khandle);

	if (!--uhr->ref_num) {
		kf_rbtree_remove(&proc->uhandle_map, node);
	}
}

static bool _parp_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y) {
	const hn_parp_t *_x = (const hn_parp_t *)x, *_y = (const hn_parp_t *)y;

	return _x->addr < _y->addr;
}

static void _parp_nodefree(kf_rbtree_node_t *p) {
	mm_kfree(p);
}

static bool _uhr_uhandle_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y) {
	const ps_uhr_t *_x = (const ps_uhr_t *)x, *_y = (const ps_uhr_t *)y;

	return _x->uhandle < _y->uhandle;
}

static void _uhr_uhandle_nodefree(kf_rbtree_node_t *p) {
	om_close_handle(((ps_uhr_t *)p)->khandle);
}
