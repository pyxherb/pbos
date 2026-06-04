#ifndef _PBOS_PS_SEMAPHORE_HH_
#define _PBOS_PS_SEMAPHORE_HH_

#include <pbos/kd/assert.h>
#include "semaphore.h"

namespace ps {
	class semaphore_t final {
	private:
		ps_semaphore_t _mutex;

	public:
		PBOS_FORCEINLINE semaphore_t() noexcept {
			ps_init_semaphore(&_mutex);
		}

		PBOS_FORCEINLINE ~semaphore_t() {
		}

		PBOS_FORCEINLINE void read_lock() noexcept {
			ps_read_lock_semaphore(&_mutex);
		}

		PBOS_FORCEINLINE void read_unlock() noexcept {
			ps_read_unlock_semaphore(&_mutex);
		}

		PBOS_FORCEINLINE void write_lock() noexcept {
			ps_write_lock_semaphore(&_mutex);
		}

		PBOS_FORCEINLINE void write_unlock() noexcept {
			ps_write_unlock_semaphore(&_mutex);
		}

		PBOS_FORCEINLINE ps_semaphore_t &c_mutex() noexcept {
			return _mutex;
		}
	};

	class read_semaphore_guard {
	private:
		ps_semaphore_t &_semaphore;

	public:
		PBOS_FORCEINLINE read_semaphore_guard(ps_semaphore_t &rec_mutex) : _semaphore(rec_mutex) {
			ps_read_lock_semaphore(&_semaphore);
		}

		PBOS_FORCEINLINE read_semaphore_guard(ps::semaphore_t &rec_mutex) : _semaphore(rec_mutex.c_mutex()) {
			ps_read_lock_semaphore(&_semaphore);
		}

		PBOS_FORCEINLINE ~read_semaphore_guard() {
			ps_read_unlock_semaphore(&_semaphore);
		}

		read_semaphore_guard(const read_semaphore_guard &) = delete;
		read_semaphore_guard(read_semaphore_guard &&) = delete;
		read_semaphore_guard &operator=(const read_semaphore_guard &) = delete;
		read_semaphore_guard &operator=(read_semaphore_guard &&) = delete;
	};

	class write_semaphore_guard {
	private:
		ps_semaphore_t &_semaphore;

	public:
		PBOS_FORCEINLINE write_semaphore_guard(ps_semaphore_t &rec_mutex) : _semaphore(rec_mutex) {
			ps_write_lock_semaphore(&_semaphore);
		}

		PBOS_FORCEINLINE write_semaphore_guard(ps::semaphore_t &rec_mutex) : _semaphore(rec_mutex.c_mutex()) {
			ps_write_lock_semaphore(&_semaphore);
		}

		PBOS_FORCEINLINE ~write_semaphore_guard() {
			ps_write_unlock_semaphore(&_semaphore);
		}

		write_semaphore_guard(const write_semaphore_guard &) = delete;
		write_semaphore_guard(write_semaphore_guard &&) = delete;
		write_semaphore_guard &operator=(const write_semaphore_guard &) = delete;
		write_semaphore_guard &operator=(write_semaphore_guard &&) = delete;
	};
}

#endif
