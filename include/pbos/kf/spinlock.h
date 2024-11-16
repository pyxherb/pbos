#ifndef _PBOS_KF_SPINLOCK_H_
#define _PBOS_KF_SPINLOCK_H_

#include <stdbool.h>

typedef bool kf_spinlock_t;

#define kf_spinlock_islocked(lock) (lock)

void kf_spinlock_lock(kf_spinlock_t *lock);
bool kf_spinlock_trylock(kf_spinlock_t *lock);

#endif
