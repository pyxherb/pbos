#include <pbos/kd/assert.h>
#include <pbos/ps/mutex.h>
#include <pbos/ki/km/symbol.hh>

PBOS_EXTERN_C_BEGIN

void ps_init_mutex(ps_mutex_t *mtx) {
	mtx->_data.lock_thread = nullptr;
	mtx->_data.spinlock = HAL_SPINLOCK_UNLOCKED;
}
KI_EXPORT_IMAGE_SYMBOL(ps_init_mutex);

void ps_lock_mutex(ps_mutex_t *mtx) {
	while (!hal_try_lock_spinlock(&mtx->_data.spinlock))
		ps_yield_cur_thread();
	mtx->_data.lock_thread = ps_get_cur_thread();
}
KI_EXPORT_IMAGE_SYMBOL(ps_lock_mutex);

bool ps_try_lock_mutex(ps_mutex_t *mtx) {
	if (!hal_try_lock_spinlock(&mtx->_data.spinlock))
		return false;
	mtx->_data.lock_thread = ps_get_cur_thread();
	return true;
}
KI_EXPORT_IMAGE_SYMBOL(ps_try_lock_mutex);

void ps_unlock_mutex(ps_mutex_t *mtx) {
	kd_dbgcheck(mtx->_data.lock_thread == ps_get_cur_thread(), "The unlocker does not match the mutex locker");
	kd_dbgcheck(mtx->_data.spinlock == HAL_SPINLOCK_LOCKED, "The mutex is not locked");
	hal_unlock_spinlock(&mtx->_data.spinlock);
}
KI_EXPORT_IMAGE_SYMBOL(ps_unlock_mutex);

bool ps_is_mutex_locked(ps_mutex_t *mtx) {
	return hal_is_spinlock_locked(&mtx->_data.spinlock);
}
KI_EXPORT_IMAGE_SYMBOL(ps_is_mutex_locked);

void ps_init_rec_mutex(ps_rec_mutex_t *mtx) {
	mtx->_data.lock_thread = nullptr;
	mtx->_data.lock_times = 0;
	mtx->_data.spinlock = HAL_SPINLOCK_UNLOCKED;
}
KI_EXPORT_IMAGE_SYMBOL(ps_init_rec_mutex);

void ps_lock_rec_mutex(ps_rec_mutex_t *mtx) {
	ps_tcb_t *cur_thread = ps_get_cur_thread();
	while (true) {
		if (hal_try_lock_spinlock(&mtx->_data.spinlock)) {
			mtx->_data.lock_thread = cur_thread;
			++mtx->_data.lock_times;
			break;
		} else if (mtx->_data.lock_thread != cur_thread) {
			ps_yield_cur_thread();
		} else {
			++mtx->_data.lock_times;
			return;
		}
	}
}
KI_EXPORT_IMAGE_SYMBOL(ps_lock_rec_mutex);

bool ps_try_lock_rec_mutex(ps_rec_mutex_t *mtx) {
	ps_tcb_t *cur_thread = ps_get_cur_thread();
	if (hal_try_lock_spinlock(&mtx->_data.spinlock)) {
		mtx->_data.lock_thread = cur_thread;
		++mtx->_data.lock_times;
	} else if (mtx->_data.lock_thread != cur_thread) {
		return false;
	}
	return true;
}
KI_EXPORT_IMAGE_SYMBOL(ps_try_lock_rec_mutex);

void ps_unlock_rec_mutex(ps_rec_mutex_t *mtx) {
	kd_dbgcheck(mtx->_data.lock_thread == ps_get_cur_thread(), "The unlocker does not match the mutex locker");
	kd_dbgcheck(mtx->_data.lock_times, "The recursive mutex is not locked");
	if (!--mtx->_data.lock_times) {
		mtx->_data.lock_thread = nullptr;
		hal_unlock_spinlock(&mtx->_data.spinlock);
	}
}
KI_EXPORT_IMAGE_SYMBOL(ps_unlock_rec_mutex);

bool ps_is_rec_mutex_locked(ps_rec_mutex_t *mtx) {
	return hal_is_spinlock_locked(&mtx->_data.spinlock);
}
KI_EXPORT_IMAGE_SYMBOL(ps_is_rec_mutex_locked);

size_t ps_get_rec_mutex_lock_times(ps_rec_mutex_t *mtx) {
	kd_dbgcheck(mtx->_data.spinlock == HAL_SPINLOCK_LOCKED, "The mutex is not locked");
	kd_dbgcheck(mtx->_data.lock_thread == ps_get_cur_thread(), "Only recursive mutex locker can get lock times");
	return mtx->_data.lock_times;
}
KI_EXPORT_IMAGE_SYMBOL(ps_get_rec_mutex_lock_times);

PBOS_EXTERN_C_END
