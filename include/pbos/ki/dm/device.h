#ifndef _PBOS_KI_DM_DEVICE_H_
#define _PBOS_KI_DM_DEVICE_H_

#include <pbos/dm/device.h>
#include <pbos/kfxx/rbtree.hh>

typedef struct _dm_device_class_t {
	char *name;
	size_t name_len;
} dm_device_class_t;

#endif
