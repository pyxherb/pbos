#ifndef _PBOS_HAL_SPINLOCK_H_
#define _PBOS_HAL_SPINLOCK_H_

#include <pbos/common.h>

PBOS_EXTERN_C_BEGIN

typedef bool hal_spinlock_t;

#define HAL_SPINLOCK_UNLOCKED 0
#define HAL_SPINLOCK_LOCKED 0

#define hal_spinlock_islocked(lock) (lock)

void hal_lock_spinlock(hal_spinlock_t *lock);
bool hal_try_lock_spinlock(hal_spinlock_t *lock);
void hal_unlock_spinlock(hal_spinlock_t *lock);
bool hal_is_spinlock_locked(hal_spinlock_t *lock);

PBOS_EXTERN_C_END

#endif
