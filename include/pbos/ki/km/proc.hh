#ifndef _PBOS_KI_KM_PROC_HH_
#define _PBOS_KI_KM_PROC_HH_

#include <pbos/ps/proc.h>
#include <pbos/ps/sched.h>
#include <pbos/se/user.h>
#include <pbos/kfxx/rbtree.hh>
#include <pbos/kfxx/allocator.hh>
#include <pbos/fs/file.hh>

PBOS_EXTERN_C_BEGIN

typedef struct _ps_ufcb_t : public kfxx::rbtree_t<ps_ufd_t>::node_t {
	fs_fcb_t *kernel_fcb;

	_ps_ufcb_t() = default;
	~_ps_ufcb_t() = default;
} ps_ufcb_t;

typedef struct _ps_tcb_t : public kfxx::rbtree_t<ps_thread_id_t>::node_t {
	ps_pcb_t *parent = nullptr;

	uint8_t priority, flags;

	kh_user_context_t *context = nullptr;
	void *stack = nullptr;
	size_t stack_size = 0;

	void *kernel_stack = nullptr;
	size_t kernel_stack_size = 0;

	se_uid_t user_id = SE_UID_ANY;

	_ps_tcb_t() = default;
	~_ps_tcb_t() = default;
} ps_tcb_t;

typedef struct _ps_par_t : kfxx::rbtree_t<void *>::node_t {
} ps_par_t;

typedef struct _ps_pcb_t : kfxx::rbtree_t<ps_proc_id_t>::node_t {
	ps_thread_id_t last_thread_id;
	kfxx::rbtree_t<void *> parp_list;
	mm_context_t *mm_context;
	kfxx::rbtree_t<void *> page_association_records;
	kfxx::rbtree_t<ps_thread_id_t> thread_set;
	uint8_t priority, flags;

	fs::fnode_ptr_t cur_dir;

	kfxx::rbtree_t<ps_ufd_t> ufcb_set;
	ps_ufd_t last_fd;

	_ps_pcb_t() = default;
	~_ps_pcb_t() = default;
} ps_pcb_t;

extern kfxx::rbtree_t<ps_proc_id_t> ps_global_proc_set;
extern ps_sched_t ps_simploop_sched;

void hal_prepare_ps();
void ps_init();

PBOS_NORETURN void kh_enter_sched(ps_cpu_id_t cpuid);

void ki_destroy_proc(ps_pcb_t *pcb);
void ki_destroy_thread(ps_tcb_t *tcb);

ps_proc_id_t ki_alloc_proc_id();

void ps_thread_set_entry(ps_tcb_t *tcb, void *ptr);
void ki_thread_set_stack(ps_tcb_t *tcb, void *ptr, size_t size);
void ki_thread_set_kernel_stack(ps_tcb_t *tcb, void *ptr, size_t size);

void ki_ps_assoc_page(ps_pcb_t *pcb, void *paddr);
void ki_ps_deassoc_page(ps_pcb_t *pcb, void *paddr);

void ki_switch_to_user_process(ps_pcb_t *pcb);
PBOS_NORETURN void ki_switch_to_user_thread(ps_tcb_t *tcb);
PBOS_NORETURN void ki_switch_to_kernel_thread(ps_tcb_t *tcb);

PBOS_EXTERN_C_END

#endif
