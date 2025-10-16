#ifndef _PBOS_HAL_SPINLOCK_H_
#define _PBOS_HAL_SPINLOCK_H_

#include <pbos/common.h>

PBOS_EXTERN_C_BEGIN

typedef bool hal_spinlock_t;

#define HAL_SPINLOCK_DEFAULT_VALUE 0

#define hal_spinlock_islocked(lock) (lock)

void hal_spinlock_lock(hal_spinlock_t *lock);
bool hal_spinlock_trylock(hal_spinlock_t *lock);

PBOS_EXTERN_C_END

#endif
