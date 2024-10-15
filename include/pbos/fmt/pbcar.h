#ifndef _PBOS_FMT_OICAR_H_
#define _PBOS_FMT_OICAR_H_

#include <pbos/common.h>
#include <stdbool.h>
#include <stdint.h>

#define OICAR_MAGIC_0 0x69
#define OICAR_MAGIC_1 0x63
#define OICAR_MAGIC_2 0x61
#define OICAR_MAGIC_3 0x72

#define OICAR_FILE_FLAG_LOAD 0x01
#define OICAR_FILE_FLAG_REQUIRED 0x02
#define OICAR_FILE_FLAG_KEEP 0x04
#define OICAR_FILE_FLAG_END 0x80

#ifdef __cplusplus
extern "C" {
#endif

#define OICAR_METADATA_BE 0x01	 // Big-endian

typedef struct _pbcar_metadata_t {
	uint8_t magic[4];
	uint8_t flags;
	uint8_t major_ver;
	uint8_t minor_ver;
	uint8_t reserved[24];
} pbcar_metadata_t;

typedef struct _pbcar_fentry_t {
	char filename[32];
	uint8_t flags;
	uint8_t reserved[11];
	uint32_t size;
} pbcar_fentry_t;

#ifdef __cplusplus
}
#endif

#endif
