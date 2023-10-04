#ifndef HAL_I386_MM_SKID_H_
#define HAL_I386_MM_SKID_H_

#include "../mm.h"
#include "skid/pchunk.h"
#include "skid/vchunk.h"
#include "skid/uchunk.h"
#include "skid/frag.h"
#include "skid/dbg.h"

void *skid_vmalloc(size_t size);

#endif
