#ifndef _PBOS_PS_MUTEX_H_
#define _PBOS_PS_MUTEX_H_

#include <pbos/hal/spinlock.h>
#include "proc.h"

#ifndef __cplusplus
	#include <stdalign.h>
#endif

PBOS_EXTERN_C_BEGIN

struct _ps_mutex_data_t {
	ps_tcb_t *lock_thread;
	hal_spinlock_t spinlock;
};

typedef struct _ps_mutex_t {
	struct _ps_mutex_data_t _data;
	char _reserved[32 - sizeof(struct _ps_mutex_data_t)];
} ps_mutex_t;

void ps_init_mutex(ps_mutex_t *mtx);
void ps_lock_mutex(ps_mutex_t *mtx);
bool ps_try_lock_mutex(ps_mutex_t *mtx);
void ps_unlock_mutex(ps_mutex_t *mtx);

PBOS_EXTERN_C_END

#endif
