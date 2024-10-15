#ifndef _PBOS_KM_INITCAR_H_
#define _PBOS_KM_INITCAR_H_

#include <hal/i386/mm.h>
#include <pbos/fmt/pbcar.h>
#include <pbos/fs/fs.h>
#include <pbos/fs/file.h>

#define INITCAR_UUID UUID(44417a9f, 01be, fd99, 93d2, 010a9fc70042)

typedef struct _initcar_file_exdata_t {
	const char *ptr;
	size_t sz_total;
	size_t name_len;
	char name[];
} initcar_file_exdata_t;

km_result_t initcar_open(fs_file_t *file, om_handle_t* handle_out);
km_result_t initcar_close(om_handle_t handle);
km_result_t initcar_read(fs_file_t *file, char *dest, size_t size, size_t off, size_t *bytes_read_out);
km_result_t initcar_write(fs_file_t *file, const char *src, size_t size, size_t off, size_t *bytes_written_out);
km_result_t initcar_size(fs_file_t *file, size_t *size_out);
km_result_t initcar_premount(fs_file_t * parent, fs_file_t * file_handle);
km_result_t initcar_postmount(fs_file_t * parent, fs_file_t * file_handle);
void initcar_mountfail(fs_file_t * parent, fs_file_t * file_handle);
km_result_t initcar_unmount(fs_file_t * file_handle);
km_result_t initcar_destructor();

void initcar_init();
void initcar_deinit();

extern fs_filesys_t *initcar_fs;
extern om_handle_t initcar_dir_handle;
extern fs_fsops_t initcar_ops;

#endif
