#include <pbos/hal/spinlock.h>
#include <arch/i386/atomic.h>

void hal_spinlock_lock(hal_spinlock_t *lock) {
	while(arch_cmpxchg8((uint8_t*)lock, 0, 1));
}

bool hal_spinlock_trylock(hal_spinlock_t *lock) {
	return arch_cmpxchg8((uint8_t*)lock, 0, 1) == 0;
}
