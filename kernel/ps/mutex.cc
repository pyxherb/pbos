#include <pbos/km/assert.h>
#include <pbos/ps/mutex.h>

PBOS_EXTERN_C_BEGIN

void ps_init_mutex(ps_mutex_t *mtx) {
	mtx->_data.lock_thread = nullptr;
	mtx->_data.spinlock = HAL_SPINLOCK_UNLOCKED;
}

void ps_lock_mutex(ps_mutex_t *mtx) {
	while (!hal_try_lock_spinlock(&mtx->_data.spinlock))
		ps_yield_cur_thread();
	mtx->_data.lock_thread = ps_get_cur_thread();
}

bool ps_try_lock_mutex(ps_mutex_t *mtx) {
	if (!hal_try_lock_spinlock(&mtx->_data.spinlock))
		return false;
	mtx->_data.lock_thread = ps_get_cur_thread();
	return true;
}

void ps_unlock_mutex(ps_mutex_t *mtx) {
	kd_dbgcheck(mtx->_data.lock_thread == ps_get_cur_thread(), "The unlocker does not match the mutex locker");
	kd_dbgcheck(mtx->_data.spinlock == HAL_SPINLOCK_LOCKED, "The mutex is not locked");
	hal_unlock_spinlock(&mtx->_data.spinlock);
}

PBOS_EXTERN_C_END
