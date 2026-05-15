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
	char _reserved[40 - sizeof(struct _ps_mutex_data_t)];
} ps_mutex_t;

struct _ps_rec_mutex_data_t {
	ps_tcb_t *lock_thread;
	size_t lock_times;
	hal_spinlock_t spinlock;
};

typedef struct _ps_rec_mutex_t {
	struct _ps_rec_mutex_data_t _data;
	char _reserved[40 - sizeof(struct _ps_mutex_data_t)];
} ps_rec_mutex_t;

struct _ps_rw_mutex_data_t {
	ps_tcb_t *lock_thread;
	size_t read_count;
	hal_spinlock_t spinlock;
	bool is_writing;
};

typedef struct _ps_rw_mutex_t {
	struct _ps_rw_mutex_data_t _data;
	char _reserved[40 - sizeof(struct _ps_rw_mutex_data_t)];
} ps_rw_mutex_t;

PBOS_API void ps_init_mutex(ps_mutex_t *mtx);
PBOS_API void ps_lock_mutex(ps_mutex_t *mtx);
PBOS_API bool ps_try_lock_mutex(ps_mutex_t *mtx);
PBOS_API void ps_unlock_mutex(ps_mutex_t *mtx);
PBOS_API bool ps_is_mutex_locked(ps_mutex_t *lock);

PBOS_API void ps_init_rec_mutex(ps_rec_mutex_t *mtx);
PBOS_API void ps_lock_rec_mutex(ps_rec_mutex_t *mtx);
PBOS_API bool ps_try_lock_rec_mutex(ps_rec_mutex_t *mtx);
PBOS_API void ps_unlock_rec_mutex(ps_rec_mutex_t *mtx);
PBOS_API bool ps_is_rec_mutex_locked(ps_rec_mutex_t *lock);
PBOS_API size_t ps_get_rec_mutex_lock_times(ps_rec_mutex_t *lock);

PBOS_API void ps_init_rw_mutex(ps_rw_mutex_t *mtx);
PBOS_API void ps_read_lock_rw_mutex(ps_rw_mutex_t *mtx);
PBOS_API void ps_write_lock_rw_mutex(ps_rw_mutex_t *mtx);
PBOS_API void ps_read_unlock_rw_mutex(ps_rw_mutex_t *mtx);
PBOS_API void ps_write_unlock_rw_mutex(ps_rw_mutex_t *mtx);

PBOS_EXTERN_C_END

#endif
