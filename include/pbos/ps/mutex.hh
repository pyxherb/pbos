#ifndef _PBOS_PS_MUTEX_HH_
#define _PBOS_PS_MUTEX_HH_

#include <pbos/kd/assert.h>
#include "mutex.h"

namespace ps {
	class Mutex final {
	private:
		ps_mutex_t _mutex;

	public:
		PBOS_FORCEINLINE Mutex() noexcept {
			ps_init_mutex(&_mutex);
		}

		PBOS_FORCEINLINE ~Mutex() {
			kd_assert(!ps_is_mutex_locked(&_mutex));
		}

		PBOS_FORCEINLINE void lock() noexcept {
			ps_lock_mutex(&_mutex);
		}

		PBOS_FORCEINLINE void unlock() noexcept {
			ps_unlock_mutex(&_mutex);
		}

		PBOS_FORCEINLINE bool try_lock() noexcept {
			return ps_try_lock_mutex(&_mutex);
		}

		PBOS_FORCEINLINE ps_mutex_t &c_mutex() noexcept {
			return _mutex;
		}
	};

	class RecMutex final {
	private:
		ps_rec_mutex_t _mutex;

	public:
		PBOS_FORCEINLINE RecMutex() noexcept {
			ps_init_rec_mutex(&_mutex);
		}

		PBOS_FORCEINLINE ~RecMutex() {
			kd_assert(!ps_is_rec_mutex_locked(&_mutex));
		}

		PBOS_FORCEINLINE void lock() noexcept {
			ps_lock_rec_mutex(&_mutex);
		}

		PBOS_FORCEINLINE void unlock() noexcept {
			ps_unlock_rec_mutex(&_mutex);
		}

		PBOS_FORCEINLINE bool try_lock() noexcept {
			return ps_try_lock_rec_mutex(&_mutex);
		}

		PBOS_FORCEINLINE ps_rec_mutex_t &c_mutex() noexcept {
			return _mutex;
		}
	};

	class RwMutex final {
	private:
		ps_rw_mutex_t _mutex;

	public:
		PBOS_FORCEINLINE RwMutex() noexcept {
			ps_init_rw_mutex(&_mutex);
		}

		PBOS_FORCEINLINE ~RwMutex() {
		}

		PBOS_FORCEINLINE void read_lock() noexcept {
			ps_read_lock_rw_mutex(&_mutex);
		}

		PBOS_FORCEINLINE void read_unlock() noexcept {
			ps_read_unlock_rw_mutex(&_mutex);
		}

		PBOS_FORCEINLINE ps_rw_mutex_t &c_mutex() noexcept {
			return _mutex;
		}
	};

	class MutexGuard {
	private:
		ps_mutex_t &_mutex;

	public:
		PBOS_FORCEINLINE MutexGuard(ps_mutex_t &rec_mutex) : _mutex(rec_mutex) {
			ps_lock_mutex(&_mutex);
		}

		PBOS_FORCEINLINE ~MutexGuard() {
			ps_unlock_mutex(&_mutex);
		}

		MutexGuard(const MutexGuard &) = delete;
		MutexGuard(MutexGuard &&) = delete;
		MutexGuard &operator=(const MutexGuard &) = delete;
		MutexGuard &operator=(MutexGuard &&) = delete;
	};

	class RecMutexGuard {
	private:
		ps_rec_mutex_t &_rec_mutex;
		size_t _lock_times;

	public:
		PBOS_FORCEINLINE RecMutexGuard(ps_rec_mutex_t &rec_mutex) : _rec_mutex(rec_mutex) {
			ps_lock_rec_mutex(&_rec_mutex);
			_lock_times = ps_get_rec_mutex_lock_times(&rec_mutex);
		}

		PBOS_FORCEINLINE ~RecMutexGuard() {
			kd_dbgcheck(_lock_times == ps_get_rec_mutex_lock_times(&_rec_mutex), "The lock times does not match the previous");
			ps_unlock_rec_mutex(&_rec_mutex);
		}

		RecMutexGuard(const RecMutexGuard &) = delete;
		RecMutexGuard(RecMutexGuard &&) = delete;
		RecMutexGuard &operator=(const RecMutexGuard &) = delete;
		RecMutexGuard &operator=(RecMutexGuard &&) = delete;
	};

	class ReadRwMutexGuard {
	private:
		ps_rw_mutex_t &_rw_mutex;

	public:
		PBOS_FORCEINLINE ReadRwMutexGuard(ps_rw_mutex_t &rec_mutex) : _rw_mutex(rec_mutex) {
			ps_read_lock_rw_mutex(&_rw_mutex);
		}

		PBOS_FORCEINLINE ~ReadRwMutexGuard() {
			ps_read_unlock_rw_mutex(&_rw_mutex);
		}

		ReadRwMutexGuard(const ReadRwMutexGuard &) = delete;
		ReadRwMutexGuard(ReadRwMutexGuard &&) = delete;
		ReadRwMutexGuard &operator=(const ReadRwMutexGuard &) = delete;
		ReadRwMutexGuard &operator=(ReadRwMutexGuard &&) = delete;
	};

	class WriteRwMutexGuard {
	private:
		ps_rw_mutex_t &_rw_mutex;

	public:
		PBOS_FORCEINLINE WriteRwMutexGuard(ps_rw_mutex_t &rec_mutex) : _rw_mutex(rec_mutex) {
			ps_write_lock_rw_mutex(&_rw_mutex);
		}

		PBOS_FORCEINLINE ~WriteRwMutexGuard() {
			ps_write_unlock_rw_mutex(&_rw_mutex);
		}

		WriteRwMutexGuard(const WriteRwMutexGuard &) = delete;
		WriteRwMutexGuard(WriteRwMutexGuard &&) = delete;
		WriteRwMutexGuard &operator=(const WriteRwMutexGuard &) = delete;
		WriteRwMutexGuard &operator=(WriteRwMutexGuard &&) = delete;
	};
}

#endif
