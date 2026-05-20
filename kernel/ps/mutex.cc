#include <pbos/kd/assert.h>
#include <pbos/kf/atomic.h>
#include <pbos/ps/mutex.h>
#include <pbos/ki/km/symbol.hh>

PBOS_EXTERN_C_BEGIN

PBOS_KERNEL_PUBLIC void ps_init_mutex(ps_mutex_t *mtx) {
	mtx->_data.lock_thread = nullptr;
	mtx->_data.spinlock = HAL_SPINLOCK_UNLOCKED;
}

PBOS_KERNEL_PUBLIC void ps_lock_mutex(ps_mutex_t *mtx) {
	while (!hal_try_lock_spinlock(&mtx->_data.spinlock))
		ps_yield_cur_thread();
	mtx->_data.lock_thread = ps_get_cur_thread();
}

PBOS_KERNEL_PUBLIC bool ps_try_lock_mutex(ps_mutex_t *mtx) {
	if (!hal_try_lock_spinlock(&mtx->_data.spinlock))
		return false;
	mtx->_data.lock_thread = ps_get_cur_thread();
	return true;
}

PBOS_KERNEL_PUBLIC void ps_unlock_mutex(ps_mutex_t *mtx) {
	kd_dbgcheck(mtx->_data.lock_thread == ps_get_cur_thread(), "The unlocker does not match the mutex locker");
	kd_dbgcheck(mtx->_data.spinlock == HAL_SPINLOCK_LOCKED, "The mutex is not locked");
	hal_unlock_spinlock(&mtx->_data.spinlock);
}

PBOS_KERNEL_PUBLIC bool ps_is_mutex_locked(ps_mutex_t *mtx) {
	return hal_is_spinlock_locked(&mtx->_data.spinlock);
}

PBOS_KERNEL_PUBLIC void ps_init_rec_mutex(ps_rec_mutex_t *mtx) {
	mtx->_data.lock_thread = nullptr;
	mtx->_data.lock_times = 0;
	mtx->_data.spinlock = HAL_SPINLOCK_UNLOCKED;
}

PBOS_KERNEL_PUBLIC void ps_lock_rec_mutex(ps_rec_mutex_t *mtx) {
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

PBOS_KERNEL_PUBLIC bool ps_try_lock_rec_mutex(ps_rec_mutex_t *mtx) {
	ps_tcb_t *cur_thread = ps_get_cur_thread();
	if (hal_try_lock_spinlock(&mtx->_data.spinlock)) {
		mtx->_data.lock_thread = cur_thread;
		++mtx->_data.lock_times;
	} else if (mtx->_data.lock_thread != cur_thread) {
		return false;
	}
	return true;
}

PBOS_KERNEL_PUBLIC void ps_unlock_rec_mutex(ps_rec_mutex_t *mtx) {
	kd_dbgcheck(mtx->_data.lock_thread == ps_get_cur_thread(), "The unlocker does not match the mutex locker");
	kd_dbgcheck(mtx->_data.lock_times, "The recursive mutex is not locked");
	if (!--mtx->_data.lock_times) {
		mtx->_data.lock_thread = nullptr;
		hal_unlock_spinlock(&mtx->_data.spinlock);
	}
}

PBOS_KERNEL_PUBLIC bool ps_is_rec_mutex_locked(ps_rec_mutex_t *mtx) {
	return hal_is_spinlock_locked(&mtx->_data.spinlock);
}

PBOS_KERNEL_PUBLIC size_t ps_get_rec_mutex_lock_times(ps_rec_mutex_t *mtx) {
	kd_dbgcheck(mtx->_data.spinlock == HAL_SPINLOCK_LOCKED, "The mutex is not locked");
	kd_dbgcheck(mtx->_data.lock_thread == ps_get_cur_thread(), "Only recursive mutex locker can get lock times");
	return mtx->_data.lock_times;
}

PBOS_KERNEL_PUBLIC void ps_init_rw_mutex(ps_rw_mutex_t *mtx) {
	mtx->_data.lock_thread = nullptr;
	mtx->_data.spinlock = HAL_SPINLOCK_UNLOCKED;
	mtx->_data.read_count = 0;
}

PBOS_KERNEL_PUBLIC void ps_read_lock_rw_mutex(ps_rw_mutex_t *mtx) {
	while (kf_atomic_cmp_xchg_u8((uint8_t *)&mtx->_data.is_writing, 0, 0) != 0)
		ps_yield_cur_thread();
	kf_atomic_inc_size(&mtx->_data.read_count);
}

PBOS_KERNEL_PUBLIC void ps_write_lock_rw_mutex(ps_rw_mutex_t *mtx) {
	kd_dbgcheck(kf_atomic_xchg_u8((uint8_t *)&mtx->_data.is_writing, true) != true, "Trying to lock an RW mutex that is already write locked");
	while (kf_atomic_cmp_xchg_size(&mtx->_data.read_count, 0, 0) != 0)
		ps_yield_cur_thread();
}

PBOS_KERNEL_PUBLIC void ps_read_unlock_rw_mutex(ps_rw_mutex_t *mtx) {
	kf_atomic_dec_size(&mtx->_data.read_count);
}

PBOS_KERNEL_PUBLIC void ps_write_unlock_rw_mutex(ps_rw_mutex_t *mtx) {
	kd_dbgcheck(kf_atomic_xchg_u8((uint8_t *)&mtx->_data.is_writing, false) != false, "Trying to unlock an RW mutex that is not write locked");
}

PBOS_EXTERN_C_END
