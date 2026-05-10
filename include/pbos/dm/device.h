#ifndef _PBOS_DM_DEVICE_H_
#define _PBOS_DM_DEVICE_H_

#include <pbos/kf/uuid.h>
#include <pbos/km/result.h>
#include <pbos/fs/file.h>

typedef struct _dm_device_t dm_device_t;

typedef struct _dm_device_class_t dm_device_class_t;

typedef struct _dm_device_ops_t {
	/// @brief Open a file.
	km_result_t (*open)(fs_fnode_t *file, fs_fcb_t **fcb_out);
	/// @brief Close a FCB.
	void (*close)(fs_fcb_t *fcb);
	/// @brief Read data from a file.
	km_result_t (*read)(fs_fcb_t *fcb, char *dest, size_t size, size_t off, size_t *bytes_read_out);
	/// @brief Write data into a file.
	km_result_t (*write)(fs_fcb_t *fcb, const void *src, size_t size, size_t off, size_t *bytes_written_out);
} dm_device_ops_t;

#endif
