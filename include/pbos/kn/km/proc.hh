#ifndef _PBOS_KN_KM_PROC_HH_
#define _PBOS_KN_KM_PROC_HH_

#include <pbos/km/proc.h>
#include <pbos/kfxx/rbtree.hh>
#include <pbos/se/user.h>

PBOS_EXTERN_C_BEGIN

typedef struct _ps_ufcb_t : public kfxx::rbtree_t<ps_ufd_t>::node_t {
	fs_fcb_t *kernel_fcb;

	_ps_ufcb_t() = default;
	~_ps_ufcb_t() = default;
} ps_ufcb_t;

typedef struct _ps_tcb_t : public kfxx::rbtree_t<thread_id_t>::node_t {
	ps_pcb_t *parent;

	uint8_t priority, flags;

	kh_user_context_t *context;
	void *stack;
	size_t stack_size;

	void *kernel_stack;
	size_t kernel_stack_size;

	se_uid_t user_id;

	_ps_tcb_t() = default;
	~_ps_tcb_t() = default;
} ps_tcb_t;

typedef struct _ps_parp_t : kfxx::rbtree_t<void *>::node_t {
	_ps_parp_t() = default;
	~_ps_parp_t() = default;
} ps_parp_t;

typedef struct _ps_pcb_t : kfxx::rbtree_t<proc_id_t>::node_t {
	thread_id_t last_thread_id;
	kfxx::rbtree_t<void*> parp_list;
	mm_context_t *mm_context;
	kfxx::rbtree_t<thread_id_t> thread_set;
	uint8_t priority, flags;

	fs_fnode_t *cur_dir;

	kfxx::rbtree_t<ps_ufd_t> ufcb_set;
	ps_ufd_t last_fd;

	_ps_pcb_t() = default;
	~_ps_pcb_t() = default;
} ps_pcb_t;

extern kfxx::rbtree_t<proc_id_t> ps_global_proc_set;
extern ps_sched_t ps_simploop_sched;

void hal_prepare_ps();
void ps_init();

PBOS_NORETURN void kn_enter_sched(ps_euid_t euid);

void kn_destroy_proc(ps_pcb_t *pcb);
void kn_destroy_thread(ps_tcb_t *tcb);

proc_id_t kn_alloc_proc_id();

void ps_thread_set_entry(ps_tcb_t *tcb, void *ptr);
void kn_thread_set_stack(ps_tcb_t *tcb, void *ptr, size_t size);
void kn_thread_set_kernel_stack(ps_tcb_t *tcb, void *ptr, size_t size);

void kn_proc_addparp(ps_pcb_t *pcb, void *paddr, uint8_t order);
void kn_proc_delparp(ps_pcb_t *pcb, void *paddr, uint8_t order);

void kn_switch_to_user_process(ps_pcb_t *pcb);
PBOS_NORETURN void kn_switch_to_user_thread(ps_tcb_t *tcb);
PBOS_NORETURN void kn_switch_to_kernel_thread(ps_tcb_t *tcb);

PBOS_EXTERN_C_END

#endif
