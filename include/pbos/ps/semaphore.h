#ifndef _PBOS_PS_SEMAPHORE_H_
#define _PBOS_PS_SEMAPHORE_H_

#include <pbos/hal/spinlock.h>
#include "proc.h"

#ifndef __cplusplus
	#include <stdalign.h>
#endif

PBOS_EXTERN_C_BEGIN

struct _ps_semaphore_data_t {
	ps_tcb_t *lock_thread;
	size_t read_count;
	hal_spinlock_t spinlock;
	bool is_writing;
};

typedef struct _ps_semaphore_t {
	struct _ps_semaphore_data_t _data;
	char _reserved[40 - sizeof(struct _ps_semaphore_data_t)];
} ps_semaphore_t;

PBOS_API void ps_init_semaphore(ps_semaphore_t *mtx);
PBOS_API void ps_read_lock_semaphore(ps_semaphore_t *mtx);
PBOS_API void ps_write_lock_semaphore(ps_semaphore_t *mtx);
PBOS_API void ps_read_unlock_semaphore(ps_semaphore_t *mtx);
PBOS_API void ps_write_unlock_semaphore(ps_semaphore_t *mtx);

PBOS_EXTERN_C_END

#endif
