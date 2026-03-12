#include <pbos/fs/file.h>
#include <pbos/fs/fs.h>
#include <pbos/km/logger.h>
#include <hal/i386/proc.hh>
#include <pbos/hal/irq.hh>

PBOS_EXTERN_C_BEGIN

ps_pcb_t **ps_cur_proc_per_eu;
ps_tcb_t **ps_cur_thread_per_eu;

static bool _parp_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y);
static void _parp_nodefree(kf_rbtree_node_t *p);

static bool _ufcb_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y);
static void _ufcb_nodefree(kf_rbtree_node_t *p);

ps_ufd_t ps_alloc_fd(ps_pcb_t *pcb) {
	return pcb->last_fd++;
}

ps_ufcb_t *ps_alloc_ufcb(ps_pcb_t *pcb, fs_fcb_t *kernel_fcb, ps_ufd_t fd) {
	ps_ufcb_t *p = (ps_ufcb_t *)mm_kmalloc(sizeof(ps_ufcb_t), alignof(ps_ufcb_t));
	kfxx::construct_at<ps_ufcb_t>(p);
	if (!p)
		return NULL;
	p->rb_value = fd;
	p->kernel_fcb = kernel_fcb;
	ps_add_ufcb(pcb, p);
	return p;
}

void ps_add_ufcb(ps_pcb_t *pcb, ps_ufcb_t *ufcb) {
	pcb->ufcb_set.insert(ufcb);
}

void ps_remove_ufcb(ps_pcb_t *pcb, ps_ufcb_t *ufcb) {
	pcb->ufcb_set.remove(ufcb);
}

ps_ufcb_t *ps_lookup_ufcb(ps_pcb_t *pcb, ps_ufd_t fd) {
	return static_cast<ps_ufcb_t *>(pcb->ufcb_set.find(fd));
}

void kn_proc_destructor(om_object_t *obj) {
	ps_pcb_t *pcb = static_cast<ps_pcb_t *>(obj);
	hn_proc_cleanup(pcb);
	kfxx::destroy_at<ps_pcb_t>(pcb);
	mm_kfree(pcb);
}

void ps_create_proc(
	ps_pcb_t *pcb,
	proc_id_t parent) {
	io::irq_disable_lock irq_disable_lock;
	if (ps_global_proc_set.find(pcb->rb_value))
		km_panic("Trying to create a new process with PCB with PID that is already used by a process");

	ps_global_proc_set.insert(pcb);
}

ps_pcb_t *ps_alloc_pcb() {
	ps_pcb_t *proc = (ps_pcb_t *)mm_kmalloc(sizeof(ps_pcb_t), alignof(ps_pcb_t));

	if (!proc)
		return NULL;

	kfxx::construct_at<ps_pcb_t>(proc);

	if (!(proc->mm_context = (mm_context_t *)mm_kmalloc(sizeof(mm_context_t), alignof(mm_context_t)))) {
		mm_kfree(proc);
		return NULL;
	}

	if (KM_FAILED(kn_mm_init_context(proc->mm_context))) {
		mm_kfree(proc->mm_context);
		mm_kfree(proc);
		return NULL;
	}

	if (KM_FAILED(ps_cur_sched->prepare_proc(ps_cur_sched, proc))) {
		mm_kfree(proc->mm_context);
		mm_kfree(proc);
		return NULL;
	}

	om_init_object(proc, ps_proc_class, 0);

	kf_rbtree_init(
		&(proc->parp_list),
		_parp_nodecmp,
		_parp_nodefree);

	proc->last_thread_id = 0;
	proc->last_fd = 0;

	proc->flags = PROC_P;

	return proc;
}

ps_pcb_t *ps_getpcb(proc_id_t pid) {
	return static_cast<ps_pcb_t *>(ps_global_proc_set.find(pid));
}

void hn_proc_cleanup(ps_pcb_t *proc) {
	ps_cur_sched->drop_proc(ps_cur_sched, proc);
	mm_free_context(proc->mm_context);
	mm_kfree(proc->mm_context);
	proc->flags &= ~PROC_A;

	for (auto it = proc->thread_set.begin(); it != proc->thread_set.end(); ++it) {
		om_decref(static_cast<ps_tcb_t *>(it.node));
	}

	kf_rbtree_free(&(proc->parp_list));
}

void ps_add_thread(ps_pcb_t *proc, ps_tcb_t *thread) {
	io::irq_disable_lock irq_disable_lock;
	// stub: do some checks with the new thread id, such as checking if a thread with the id exists.
	thread->rb_value = ++proc->last_thread_id;
	proc->thread_set.insert(thread);
}

void kn_switch_to_user_process(ps_pcb_t *pcb) {
	mm_switch_context(pcb->mm_context);
}

void kn_switch_to_user_thread(ps_tcb_t *tcb) {
	ps_load_user_context(tcb->context);
}

void kn_switch_to_kernel_thread(ps_tcb_t *tcb) {
	ps_load_kernel_context(tcb->context);
}

ps_euid_t ps_get_cur_euid() {
	return arch_storefs();
}

void kn_set_cur_euid(ps_euid_t euid) {
	arch_loadfs(euid);
}

static bool _parp_nodecmp(const kf_rbtree_node_t *x, const kf_rbtree_node_t *y) {
	const hn_parp_t *_x = (const hn_parp_t *)x, *_y = (const hn_parp_t *)y;

	return _x->addr < _y->addr;
}

static void _parp_nodefree(kf_rbtree_node_t *p) {
	mm_kfree(p);
}

PBOS_EXTERN_C_END
