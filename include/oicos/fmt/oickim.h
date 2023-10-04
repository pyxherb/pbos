#ifndef _OICOS_FMT_OICKIM_H_
#define _OICOS_FMT_OICKIM_H_

#include <stdint.h>
#include <stdbool.h>
#include <oicos/common.h>

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

__packed_begin

typedef struct __packed _oickim_ihdr_t {
	uint8_t magic[4];
	bool endian;
	uint16_t machine;
	uint8_t flags;
	uint64_t image_size;
} oickim_ihdr_t;

__packed_end

#ifdef __cplusplus
}
#endif

#endif
