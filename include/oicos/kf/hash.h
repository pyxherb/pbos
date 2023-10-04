///
/// @file hash.h
/// @brief Header file for the hash facility module.
///
/// @copyright Copyright (c) 2023 OicOS Contributors
///
#ifndef _OICOS_KF_HASH_H_
#define _OICOS_KF_HASH_H_

#include <stdint.h>
#include <stddef.h>

uint64_t kf_hash_djb(const char *src, size_t size);

#endif
