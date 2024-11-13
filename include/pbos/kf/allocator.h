#ifndef _PBOS_KF_ALLOCATOR_H_
#define _PBOS_KF_ALLOCATOR_H_

#include <pbos/utils.h>

typedef void *(*kf_allocator_alloc_t)(size_t size);
typedef void *(*kf_allocator_dealloc_t)(void *ptr);

typedef struct _kf_allocator_t {
	kf_allocator_alloc_t alloc;
	kf_allocator_dealloc_t dealloc;
} kf_allocator_t;

#endif
