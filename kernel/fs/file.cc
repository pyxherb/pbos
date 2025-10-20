#include <pbos/kf/hash.h>
#include <pbos/kf/string.h>
#include <pbos/km/mm.h>
#include <pbos/kn/fs/file.hh>
#include <pbos/kn/fs/fs.hh>
#include <pbos/km/objmgr.hh>

km_result_t kn_alloc_file(
	fs_filesys_t *fs,
	const char *filename,
	size_t filename_len,
	fs_filetype_t filetype,
	size_t exdata_size,
	fs_file_t **file_out) {
	if (!filename_len)
		return KM_MAKEERROR(KM_RESULT_INVALID_ARGS);

	// Allocate the file object.
	fs_file_t *file = (fs_file_t *)mm_kmalloc(sizeof(fs_file_t) + exdata_size, alignof(fs_file_t));
	if (!file)
		return KM_MAKEERROR(KM_RESULT_NO_MEM);
	memset(file, 0, sizeof(fs_file_t));

	if (!(file->filename = (char *)mm_kmalloc(filename_len, sizeof(char)))) {
		mm_kfree(file);
		return KM_MAKEERROR(KM_RESULT_NO_MEM);
	}
	memcpy(file->filename, filename, filename_len);
	file->filename_len = filename_len;

	file->fs = fs;
	file->filetype = filetype;
	if (file_out)
		*file_out = file;

	om_init_object(&file->object_header, fs_file_class, 0);

	return KM_RESULT_OK;
}

km_result_t fs_create_file(
	fs_file_t *parent,
	const char *filename,
	size_t filename_len,
	fs_file_t **file_out) {
	return parent->fs->ops.create_file(parent, filename, filename_len, file_out);
}

km_result_t fs_create_dir(
	fs_file_t *parent,
	const char *filename,
	size_t filename_len,
	fs_file_t **file_out) {
	return parent->fs->ops.create_dir(parent, filename, filename_len, file_out);
}

km_result_t fs_mount_file(fs_file_t *parent, fs_file_t *file) {
	return parent->fs->ops.mount(parent, file);
}

km_result_t fs_unmount_file(fs_file_t *file) {
	return file->parent->fs->ops.unmount(file);
}

km_result_t fs_child_of(fs_file_t *file, const char *filename, size_t filename_len, fs_file_t **file_out) {
	return file->fs->ops.subnode(file, filename, filename_len, file_out);
}

km_result_t fs_resolve_path(fs_file_t *cur_dir, const char *path, size_t path_len, fs_file_t **file_out) {
	fs_file_t *file = cur_dir ? fs_abs_root_dir : cur_dir;
	km_result_t result;

	om_incref(&file->object_header);

	const char *i = path, *last_divider = path;
	fs_file_t *new_file;

	while (i - path < path_len) {
		switch (*i) {
			case '/': {
				size_t filename_len = i - last_divider;

				if (!filename_len) {
					if (last_divider == path) {
						new_file = fs_abs_root_dir;
					}
				} else if (KM_FAILED(result = fs_child_of(file, last_divider, filename_len, &new_file))) {
					om_decref(&file->object_header);
					return result;
				}

				last_divider = i + 1;
				om_decref(&file->object_header);
				om_incref(&new_file->object_header);
				file = new_file;
				break;
			}
			case '\0':
				goto end;
		}

		++i;
	}

end:;
	size_t filename_len = i - last_divider;
	if (filename_len) {
		if (KM_FAILED(result = fs_child_of(file, last_divider, filename_len, &new_file))) {
			om_decref(&file->object_header);
			return result;
		}
	}

	om_decref(&file->object_header);
	om_incref(&new_file->object_header);
	*file_out = new_file;
	return KM_RESULT_OK;
}

km_result_t fs_open(fs_file_t *base_dir, const char *path, size_t path_len, fs_fcb_t **fcb_out) {
	fs_file_t *file;
	km_result_t result;

	if (KM_FAILED(result = fs_resolve_path(base_dir, path, path_len, &file)))
		return result;

	if (KM_FAILED(result = file->fs->ops.open(file, fcb_out))) {
		om_decref(&file->object_header);
		return result;
	}

	om_decref(&file->object_header);

	return KM_RESULT_OK;
}

km_result_t fs_close(fs_fcb_t *fcb) {
	return fcb->file->fs->ops.close(fcb);
}

km_result_t fs_read(fs_fcb_t *fcb, void *dest, size_t size, size_t off, size_t *bytes_read_out) {
	return fcb->file->fs->ops.read(fcb, (char *)dest, size, off, bytes_read_out);
}

km_result_t fs_write(fs_fcb_t *fcb, const char *src, size_t size, size_t off, size_t *bytes_written_out) {
	return fcb->file->fs->ops.write(fcb, src, size, off, bytes_written_out);
}

km_result_t fs_size(fs_fcb_t *fcb, size_t *size_out) {
	return fcb->file->fs->ops.size(fcb, size_out);
}

void kn_file_destructor(om_object_t *obj) {
	fs_file_t *file = PBOS_CONTAINER_OF(fs_file_t, object_header, obj);
	file->fs->ops.offload(file);
}
