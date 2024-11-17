#ifndef _PBOS_FMT_OICKIM_H_
#define _PBOS_FMT_OICKIM_H_

#include <stdint.h>
#include <stdbool.h>
#include <pbos/common.h>

#define OICKIM_MAGIC_0 0x49
#define OICKIM_MAGIC_1 0x0c
#define OICKIM_MAGIC_2 0x0a
#define OICKIM_MAGIC_3 0x72

#ifdef __cplusplus
extern "C" {
#endif

//
// Machine IDs.
//
enum{
	OICKIM_MACHINE_INVALID = 0,
	OICKIM_MACHINE_I386
};

PB_PACKED_BEGIN

typedef struct PB_PACKED _pbkim_ihdr_t {
	uint8_t magic[4];
	bool endian;
	uint16_t machine;
	uint8_t flags;
	uint64_t image_size;
} pbkim_ihdr_t;

PB_PACKED_END

#ifdef __cplusplus
}
#endif

#endif
