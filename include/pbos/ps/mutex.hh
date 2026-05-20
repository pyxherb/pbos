#ifndef _PBOS_PS_MUTEX_HH_
#define _PBOS_PS_MUTEX_HH_

#include <pbos/kd/assert.h>
#include "mutex.h"

namespace ps {
	class mutex_t final {
	private:
		ps_mutex_t _mutex;

	public:
		PBOS_FORCEINLINE mutex_t() noexcept {
			ps_init_mutex(&_mutex);
		}

		PBOS_FORCEINLINE ~mutex_t() {
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

	class rec_mutex_t final {
	private:
		ps_rec_mutex_t _mutex;

	public:
		PBOS_FORCEINLINE rec_mutex_t() noexcept {
			ps_init_rec_mutex(&_mutex);
		}

		PBOS_FORCEINLINE ~rec_mutex_t() {
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

	class rw_mutex_t final {
	private:
		ps_rw_mutex_t _mutex;

	public:
		PBOS_FORCEINLINE rw_mutex_t() noexcept {
			ps_init_rw_mutex(&_mutex);
		}

		PBOS_FORCEINLINE ~rw_mutex_t() {
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

	class mutex_guard {
	private:
		ps_mutex_t &_mutex;

	public:
		PBOS_FORCEINLINE mutex_guard(ps_mutex_t &rec_mutex) : _mutex(rec_mutex) {
			ps_lock_mutex(&_mutex);
		}

		PBOS_FORCEINLINE ~mutex_guard() {
			ps_unlock_mutex(&_mutex);
		}

		mutex_guard(const mutex_guard &) = delete;
		mutex_guard(mutex_guard &&) = delete;
		mutex_guard &operator=(const mutex_guard &) = delete;
		mutex_guard &operator=(mutex_guard &&) = delete;
	};

	class rec_mutex_guard {
	private:
		ps_rec_mutex_t &_rec_mutex;
		size_t _lock_times;

	public:
		PBOS_FORCEINLINE rec_mutex_guard(ps_rec_mutex_t &rec_mutex) : _rec_mutex(rec_mutex) {
			ps_lock_rec_mutex(&_rec_mutex);
			_lock_times = ps_get_rec_mutex_lock_times(&rec_mutex);
		}

		PBOS_FORCEINLINE ~rec_mutex_guard() {
			kd_dbgcheck(_lock_times == ps_get_rec_mutex_lock_times(&_rec_mutex), "The lock times does not match the previous");
			ps_unlock_rec_mutex(&_rec_mutex);
		}

		rec_mutex_guard(const rec_mutex_guard &) = delete;
		rec_mutex_guard(rec_mutex_guard &&) = delete;
		rec_mutex_guard &operator=(const rec_mutex_guard &) = delete;
		rec_mutex_guard &operator=(rec_mutex_guard &&) = delete;
	};

	class read_rw_mutex_guard {
	private:
		ps_rw_mutex_t &_rw_mutex;

	public:
		PBOS_FORCEINLINE read_rw_mutex_guard(ps_rw_mutex_t &rec_mutex) : _rw_mutex(rec_mutex) {
			ps_read_lock_rw_mutex(&_rw_mutex);
		}

		PBOS_FORCEINLINE ~read_rw_mutex_guard() {
			ps_read_unlock_rw_mutex(&_rw_mutex);
		}

		read_rw_mutex_guard(const read_rw_mutex_guard &) = delete;
		read_rw_mutex_guard(read_rw_mutex_guard &&) = delete;
		read_rw_mutex_guard &operator=(const read_rw_mutex_guard &) = delete;
		read_rw_mutex_guard &operator=(read_rw_mutex_guard &&) = delete;
	};

	class write_rw_mutex_guard {
	private:
		ps_rw_mutex_t &_rw_mutex;

	public:
		PBOS_FORCEINLINE write_rw_mutex_guard(ps_rw_mutex_t &rec_mutex) : _rw_mutex(rec_mutex) {
			ps_write_lock_rw_mutex(&_rw_mutex);
		}

		PBOS_FORCEINLINE ~write_rw_mutex_guard() {
			ps_write_unlock_rw_mutex(&_rw_mutex);
		}

		write_rw_mutex_guard(const write_rw_mutex_guard &) = delete;
		write_rw_mutex_guard(write_rw_mutex_guard &&) = delete;
		write_rw_mutex_guard &operator=(const write_rw_mutex_guard &) = delete;
		write_rw_mutex_guard &operator=(write_rw_mutex_guard &&) = delete;
	};
}

#endif
