#include <hal/i386/proc.h>

static int _parp_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y);
static int _parp_keycmp(const kf_rbtree_node_t *x, const void *key);
static void _parp_nodefree(kf_rbtree_node_t *p);

void kn_proc_destructor(om_object_t *obj) {
	hn_proc_cleanup((ps_pcb_t *)obj);
}

ps_pcb_t *kn_alloc_pcb() {
	ps_pcb_t *proc = mm_kmalloc(sizeof(ps_pcb_t));

	if(!proc)
		return NULL;

	memset(proc, 0, sizeof(ps_pcb_t));

	if(!(proc->mmctxt = mm_create_context())) {
		mm_kfree(proc);
		return NULL;
	}

	om_init_object(&(proc->object_header), ps_proc_class);

	kf_rbtree_init(
		&(proc->parp_list),
		_parp_nodecmp,
		_parp_keycmp,
		_parp_nodefree);

	proc->flags = PROC_P;

	return proc;
}

ps_pcb_t *ps_getpcb(proc_id_t pid) {
	return &hn_proc_list[pid];
}

void hn_proc_cleanup(ps_pcb_t *proc) {
	mm_free_context(proc->mmctxt);
	proc->flags &= ~PROC_A;

	for (ps_tcb_t *i = proc->threads; i; i = CONTAINER_OF(ps_tcb_t, list_header, kf_list_next(&(i->list_header)))) {
		om_decref(&(i->object_header));
	}

	kf_rbtree_free(&(proc->parp_list));
}

mm_context_t *ps_mmcontext_of(ps_pcb_t *proc) {
	return proc->mmctxt;
}

void ps_add_thread(ps_pcb_t *proc, ps_tcb_t *thread) {
	if (proc->threads) {
		kf_list_append(&proc->threads->list_header, &thread->list_header);
	} else
		proc->threads = thread;
}

void kn_start_user_process(ps_pcb_t *pcb) {
	mm_switch_context(pcb->mmctxt);

	kn_start_user_thread(pcb->threads);
}

void kn_start_user_thread(ps_tcb_t *tcb) {
	ps_load_user_context(&tcb->context);
}

static int _parp_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y) {
	const hn_parp_t *_x = (const hn_parp_t *)x, *_y = (const hn_parp_t *)y;

	if (_x->addr > _y->addr)
		return 1;
	else if (_x->addr < _y->addr)
		return -1;

	return 0;
}

static int _parp_keycmp(const kf_rbtree_node_t *x, const void *key) {
	const hn_parp_t *_x = (const hn_parp_t *)x;
	pgaddr_t _key = (pgaddr_t)key;

	if (_x->addr > _key)
		return 1;
	else if (_x->addr < _key)
		return -1;

	return 0;
}

static void _parp_nodefree(kf_rbtree_node_t *p) {
	mm_kfree(p);
}
