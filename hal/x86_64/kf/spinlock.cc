#include <arch/x86_64/atomic.h>
#include <arch/x86_64/misc.h>
#include <pbos/hal/spinlock.h>
#include <pbos/ps/proc.h>
#include <pbos/kd/assert.h>

PBOS_EXTERN_C_BEGIN

void hal_lock_spinlock(hal_spinlock_t *lock) {
	while (arch_atomic_cmpxchg8((uint8_t *)lock, HAL_SPINLOCK_UNLOCKED, HAL_SPINLOCK_LOCKED) == HAL_SPINLOCK_LOCKED);
	// ps_yield_cur_thread();
}

bool hal_try_lock_spinlock(hal_spinlock_t *lock) {
	return arch_atomic_cmpxchg8((uint8_t *)lock, HAL_SPINLOCK_UNLOCKED, HAL_SPINLOCK_LOCKED) != HAL_SPINLOCK_LOCKED;
}

void hal_unlock_spinlock(hal_spinlock_t *lock) {
	kd_dbgcheck(hal_is_spinlock_locked(lock), "The spinlock is not locked");
	*lock = HAL_SPINLOCK_UNLOCKED;
}

bool hal_is_spinlock_locked(hal_spinlock_t *lock) {
	return *lock == HAL_SPINLOCK_LOCKED;
}

PBOS_EXTERN_C_END
