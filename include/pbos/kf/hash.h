///
/// @file hash.h
/// @brief Header file for the hash facility module.
///
/// @copyright Copyright (c) 2023 PbOS Contributors
///
#ifndef _PBOS_KF_HASH_H_
#define _PBOS_KF_HASH_H_

#include "basedefs.h"

PBOS_EXTERN_C_BEGIN

uint32_t kf_djb_hash32(const char *data, size_t size);
uint64_t kf_djb_hash64(const char *data, size_t size);
uint32_t kf_city_hash32(const char *s, size_t len);
uint64_t kf_city_hash64(const char *s, size_t len);

PBOS_EXTERN_C_END

#endif
