#ifndef _PBOS_KFXX_UUID_HH_
#define _PBOS_KFXX_UUID_HH_

#include <pbos/kf/uuid.h>

PBOS_FORCEINLINE bool operator==(const kf_uuid_t &lhs, const kf_uuid_t &rhs) noexcept {
	return uuid_eq(&lhs, &rhs);
}

PBOS_FORCEINLINE bool operator<(const kf_uuid_t &lhs, const kf_uuid_t &rhs) noexcept {
	return uuid_lt(&lhs, &rhs);
}

PBOS_FORCEINLINE bool operator>(const kf_uuid_t &lhs, const kf_uuid_t &rhs) noexcept {
	return uuid_gt(&lhs, &rhs);
}

#endif
