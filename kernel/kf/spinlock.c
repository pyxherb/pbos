#include <pbos/kf/spinlock.h>

void kf_spinlock_lock(kf_spinlock_t *lock) {
    // TODO: Use compare-and-exchange instead of the rough way.
    while(lock) {}
    *lock = true;
}

bool kf_spinlock_trylock(kf_spinlock_t *lock) {
    if(!lock) {
        return *lock = true;
    }
    return false;
}