#ifndef _PBOS_KM_PROC_HH_
#define _PBOS_KM_PROC_HH_

#include "proc.h"
#include <pbos/kfxx/rbtree.hh>

typedef struct _ps_ufcb_t : public kfxx::rbtree_t<ps_ufd_t>::node_t {
	fs_fcb_t *kernel_fcb;
} ps_ufcb_t;

typedef struct _ps_tcb_t : public kfxx::rbtree_t<thread_id_t>::node_t, public om_object_t {
	ps_pcb_t *parent;

	uint8_t priority, flags;

	ps_user_context_t *context;
	void *stack;
	size_t stack_size;

	void *kernel_stack;
	size_t kernel_stack_size;
} ps_tcb_t;

typedef struct _ps_pcb_t : kfxx::rbtree_t<proc_id_t>::node_t, public om_object_t {
	thread_id_t last_thread_id;
	kf_rbtree_t parp_list;
	mm_context_t *mm_context;
	kfxx::rbtree_t<thread_id_t> thread_set;
	uint8_t priority, flags;

	fs_file_t *cur_dir;

	kfxx::rbtree_t<ps_ufd_t> ufcb_set;
	ps_ufd_t last_fd;
} ps_pcb_t;

#endif
