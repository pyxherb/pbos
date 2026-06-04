#include <pbos/kd/assert.h>
#include <pbos/kf/atomic.h>
#include <pbos/ps/semaphore.h>
#include <pbos/ki/km/symbol.hh>

PBOS_EXTERN_C_BEGIN

PBOS_API void ps_init_semaphore(ps_semaphore_t *mtx) {
	mtx->_data.lock_thread = nullptr;
	mtx->_data.spinlock = HAL_SPINLOCK_UNLOCKED;
	mtx->_data.read_count = 0;
}

PBOS_API void ps_read_lock_semaphore(ps_semaphore_t *mtx) {
	while (kf_atomic_cmp_xchg_u8((uint8_t *)&mtx->_data.is_writing, 0, 0) != 0)
		ps_yield_cur_thread();
	kf_atomic_inc_size(&mtx->_data.read_count);
}

PBOS_API void ps_write_lock_semaphore(ps_semaphore_t *mtx) {
	kd_dbgcheck(kf_atomic_xchg_u8((uint8_t *)&mtx->_data.is_writing, true) != true, "Trying to lock an RW mutex that is already write locked");
	while (kf_atomic_cmp_xchg_size(&mtx->_data.read_count, 0, 0) != 0)
		ps_yield_cur_thread();
}

PBOS_API void ps_read_unlock_semaphore(ps_semaphore_t *mtx) {
	kf_atomic_dec_size(&mtx->_data.read_count);
}

PBOS_API void ps_write_unlock_semaphore(ps_semaphore_t *mtx) {
	kd_dbgcheck(kf_atomic_xchg_u8((uint8_t *)&mtx->_data.is_writing, false) != false, "Trying to unlock an RW mutex that is not write locked");
}

PBOS_EXTERN_C_END
