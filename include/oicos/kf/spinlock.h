#ifndef _OICOS_KF_SPINLOCK_H_
#define _OICOS_KF_SPINLOCK_H_

#include <stdbool.h>

typedef bool kf_spinlock_t;

#define kf_spinlock_lock(lock) \
	while ((lock)) {}
#define kf_spinlock_trylock(lock) ((lock) ? false : (lock = true))
#define kf_spinlock_islocked(lock) (lock)

#endif
