#include <pbos/hal/spinlock.h>
#include <pbos/ps/proc.h>
#include <arch/x86_64/atomic.h>
#include <arch/x86_64/misc.h>

PBOS_EXTERN_C_BEGIN

void hal_lock_spinlock(hal_spinlock_t *lock) {
	while(arch_cmpxchg8((uint8_t*)lock, 0, 1));
		// ps_yield_cur_thread();
}

bool hal_try_lock_spinlock(hal_spinlock_t *lock) {
	return arch_cmpxchg8((uint8_t*)lock, 0, 1) == 0;
}

void hal_unlock_spinlock(hal_spinlock_t *lock) {
	lock = HAL_SPINLOCK_UNLOCKED;
}

PBOS_EXTERN_C_END
