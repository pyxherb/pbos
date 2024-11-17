#include <pbos/kf/hash.h>
#include <pbos/kf/string.h>
#include <pbos/km/mm.h>
#include <pbos/kn/fs/file.h>
#include <pbos/kn/fs/fs.h>

typedef struct _kn_file_hasher_key_t {
	size_t filename_len;
	const char *filename;
} kn_file_hasher_key_t;

typedef struct _kn_child_file_entry_t {
	kf_hashmap_node_t hashmap_header;
	om_handle_t file_handle;
} kn_child_file_entry_t;

km_result_t fs_deref_file_handle(
	om_handle_t file_handle,
	fs_file_t **file_out) {
	om_object_t *object;
	km_result_t result;
	if (KM_FAILED(result = om_deref_handle(file_handle, &object)))
		return result;
	*file_out = PB_CONTAINER_OF(fs_file_t, object_header, object);
	return result;
}

km_result_t kn_create_file(
	fs_filesys_t *fs,
	const char *filename,
	size_t filename_len,
	fs_filetype_t filetype,
	size_t exdata_size,
	om_handle_t *handle_out,
	fs_file_t **file_out) {
	if (!filename_len)
		return KM_MAKEERROR(KM_RESULT_INVALID_ARGS);

	// Allocate the file object.
	fs_file_t *file = mm_kmalloc(sizeof(fs_file_t) + exdata_size);
	if (!file)
		return KM_MAKEERROR(KM_RESULT_NO_MEM);
	memset(file, 0, sizeof(fs_file_t));

	if (!(file->filename = mm_kmalloc(filename_len))) {
		mm_kfree(file);
		return KM_MAKEERROR(KM_RESULT_NO_MEM);
	}
	memcpy(file->filename, filename, filename_len);
	file->filename_len = filename_len;

	file->fs = fs;
	file->filetype = filetype;
	if (file_out)
		*file_out = file;

	om_init_object(&file->object_header, fs_file_class);

	km_result_t result = om_create_handle(&file->object_header, handle_out);
	if (KM_FAILED(result))
		om_gc();
	return result;
}

km_result_t fs_create_file(
	fs_filesys_t *fs,
	const char *filename,
	size_t filename_len,
	size_t exdata_size,
	om_handle_t *handle_out) {
	return kn_create_file(fs, filename, filename_len, FS_FILETYPE_FILE, exdata_size, handle_out, NULL);
}

static size_t _kn_file_hasher(size_t bucket_num, const void *target, bool is_target_key);
static void _kn_file_nodefree(kf_hashmap_node_t *node);
static bool _kn_file_nodecmp(const kf_hashmap_node_t *lhs, const kf_hashmap_node_t *rhs);
static bool _kn_file_keycmp(const kf_hashmap_node_t *lhs, const void *key);

km_result_t fs_create_dir(
	fs_filesys_t *fs,
	const char *filename,
	size_t filename_len,
	size_t exdata_size,
	om_handle_t *handle_out) {
	fs_file_t *file;
	km_result_t result = kn_create_file(fs, filename, filename_len, FS_FILETYPE_DIR, sizeof(fs_dir_exdata_t) + exdata_size, handle_out, &file);
	if (KM_FAILED(result))
		return result;

	fs_dir_exdata_t *exdata = (fs_dir_exdata_t *)fs_file_exdata(file);
	kf_hashmap_init(
		&exdata->children,
		_kn_file_hasher,
		_kn_file_nodefree,
		_kn_file_nodecmp,
		_kn_file_keycmp);

	return KM_RESULT_OK;
}

#include <pbos/km/logger.h>

static size_t _kn_file_hasher(size_t bucket_num, const void *target, bool is_target_key) {
	if (is_target_key) {
		const kn_file_hasher_key_t *key = target;
		return kf_hash_djb(key->filename, key->filename_len) % bucket_num;
	}

	kn_child_file_entry_t *registry = PB_CONTAINER_OF(kn_child_file_entry_t, hashmap_header, target);
	fs_file_t *file;
	if (KM_FAILED(fs_deref_file_handle(registry->file_handle, &file)))
		km_panic("Error deferencing the file handle");
	return kf_hash_djb(file->filename, file->filename_len) % bucket_num;
}

static void _kn_file_nodefree(kf_hashmap_node_t *node) {
	kn_child_file_entry_t *registry = PB_CONTAINER_OF(kn_child_file_entry_t, hashmap_header, node);
	om_close_handle(registry->file_handle);
}

static bool _kn_file_nodecmp(const kf_hashmap_node_t *lhs, const kf_hashmap_node_t *rhs) {
	kn_child_file_entry_t *_lhs = PB_CONTAINER_OF(kn_child_file_entry_t, hashmap_header, lhs),
						  *_rhs = PB_CONTAINER_OF(kn_child_file_entry_t, hashmap_header, rhs);

	fs_file_t *lhs_file, *rhs_file;
	if (KM_FAILED(fs_deref_file_handle(_lhs->file_handle, &lhs_file)))
		km_panic("Error deferencing the file handle");
	if (KM_FAILED(fs_deref_file_handle(_rhs->file_handle, &rhs_file)))
		km_panic("Error deferencing the file handle");

	uint64_t lhs_hash = kf_hash_djb(lhs_file->filename, lhs_file->filename_len),
			 rhs_hash = kf_hash_djb(rhs_file->filename, rhs_file->filename_len);

	return lhs_hash == rhs_hash;
}
static bool _kn_file_keycmp(const kf_hashmap_node_t *lhs, const void *key) {
	kn_child_file_entry_t *_lhs = PB_CONTAINER_OF(kn_child_file_entry_t, hashmap_header, lhs);

	fs_file_t *lhs_file;
	if (KM_FAILED(fs_deref_file_handle(_lhs->file_handle, &lhs_file)))
		km_panic("Error deferencing the file handle");

	uint64_t lhs_hash = kf_hash_djb(lhs_file->filename, lhs_file->filename_len),
			 key_hash = kf_hash_djb(((kn_file_hasher_key_t *)key)->filename, ((kn_file_hasher_key_t *)key)->filename_len);

	return lhs_hash == key_hash;
}

#include <pbos/km/logger.h>

km_result_t fs_mount_file(om_handle_t parent, om_handle_t file_handle) {
	fs_file_t *parent_file, *file;
	km_result_t result;

	if (KM_FAILED(result = fs_deref_file_handle(parent, &parent_file)))
		return result;

	if (KM_FAILED(result = fs_deref_file_handle(file_handle, &file)))
		return result;

	if (parent_file->filetype != FS_FILETYPE_DIR)
		return KM_MAKEERROR(KM_RESULT_INVALID_ARGS);

	if (KM_FAILED(result = parent_file->fs->ops.premount(parent_file, file))) {
		parent_file->fs->ops.mountfail(parent_file, file);
		return result;
	}

	fs_dir_exdata_t *exdata = (fs_dir_exdata_t *)fs_file_exdata(parent_file);
	kf_hashmap_t *hm = &exdata->children;

	kn_child_file_entry_t *cfe = mm_kmalloc(sizeof(kn_child_file_entry_t));
	if (!cfe) {
		parent_file->fs->ops.mountfail(parent_file, file);
		return KM_MAKEERROR(KM_RESULT_NO_MEM);
	}
	cfe->file_handle = file_handle;

	if (KM_FAILED(result = kf_hashmap_insert(hm, &cfe->hashmap_header)))
		return result;

	if (KM_FAILED(result = parent_file->fs->ops.postmount(parent_file, file))) {
		km_result_t remove_result = kf_hashmap_remove(hm, &cfe->hashmap_header);
		assert(KM_SUCCEEDED(remove_result));

		return result;
	}

	return KM_RESULT_OK;
}

km_result_t fs_unmount_file(om_handle_t file_handle) {
	fs_file_t *parent_file, *file;
	km_result_t result;

	if (KM_FAILED(result = fs_deref_file_handle(file_handle, &file)))
		return result;
	if (KM_FAILED(result = fs_deref_file_handle(file->parent, &parent_file)))
		return result;

	kf_hashmap_t *hm = &((fs_dir_exdata_t *)parent_file->exdata)->children;
	kn_file_hasher_key_t key = {
		.filename = file->filename,
		.filename_len = file->filename_len
	};
	kf_hashmap_node_t *node = kf_hashmap_find(hm, &key);
	if (!node)
		return KM_MAKEERROR(KM_RESULT_NOT_FOUND);

	return kf_hashmap_remove(&((fs_dir_exdata_t *)parent_file->exdata)->children, node);
}

km_result_t fs_child_of(om_handle_t file_handle, const char *name, size_t filename_len, om_handle_t *handle_out) {
	fs_file_t *file;
	km_result_t result;
	if (KM_FAILED(result = fs_deref_file_handle(file_handle, &file)))
		return result;

	switch (file->filetype) {
		case FS_FILETYPE_DIR: {
			// Copy the file name.
			kn_file_hasher_key_t k = {
				.filename = name,
				.filename_len = filename_len
			};

			kf_hashmap_node_t *node = kf_hashmap_find(&((fs_dir_exdata_t *)(file->exdata))->children, &k);
			if (!node)
				return KM_MAKEERROR(KM_RESULT_NOT_FOUND);

			*handle_out = PB_CONTAINER_OF(kn_child_file_entry_t, hashmap_header, node)->file_handle;
			break;
		}
		case FS_FILETYPE_LINK:
			// stub
		default:
			return KM_MAKEERROR(KM_RESULT_NOT_FOUND);
	}

	return KM_RESULT_OK;
}

km_result_t fs_resolve_path(om_handle_t cur_dir, const char *path, size_t path_len, om_handle_t *file_out) {
	om_handle_t file = cur_dir == OM_INVALID_HANDLE ? fs_abs_root_dir : cur_dir;
	km_result_t result;

	const char *i = path, *last_divider = path;
	while (i - path < path_len) {
		switch (*i) {
			case '/': {
				size_t filename_len = i - last_divider;

				if (!filename_len) {
					if (last_divider == path)
						file = fs_abs_root_dir;
				} else if (KM_FAILED(result = fs_child_of(file, last_divider, filename_len, &file)))
					return result;

				last_divider = i + 1;
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
		if (KM_FAILED(result = fs_child_of(file, last_divider, filename_len, &file)))
			return result;
	}

	*file_out = file;
	return KM_RESULT_OK;
}

km_result_t fs_open(const char *path, size_t path_len, om_handle_t *handle_out) {
	om_handle_t file_handle;
	fs_file_t *file;
	km_result_t result;

	if (KM_FAILED(result = fs_resolve_path(fs_abs_root_dir, path, path_len, &file_handle)))
		return result;

	if (KM_FAILED(result = fs_deref_file_handle(file_handle, &file)))
		return result;

	return file->fs->ops.open(file, handle_out);
}

km_result_t fs_close_file(om_handle_t file_handle) {
	fs_file_t *file;
	km_result_t result;

	if (KM_FAILED(result = fs_deref_file_handle(file_handle, &file)))
		return result;

	return file->fs->ops.close(file_handle);
}

km_result_t fs_read(om_handle_t file_handle, void *dest, size_t size, size_t off, size_t *bytes_read_out) {
	fs_file_t *file;
	km_result_t result;

	if (KM_FAILED(result = fs_deref_file_handle(file_handle, &file)))
		return result;

	return file->fs->ops.read(file, (char *)dest, size, off, bytes_read_out);
}

km_result_t fs_write(om_handle_t file_handle, const char *src, size_t size, size_t off, size_t *bytes_written_out) {
	fs_file_t *file;
	km_result_t result;

	if (KM_FAILED(result = fs_deref_file_handle(file_handle, &file)))
		return result;

	return file->fs->ops.write(file, src, size, off, bytes_written_out);
}

km_result_t fs_size(om_handle_t file_handle, size_t *size_out) {
	fs_file_t *file;
	km_result_t result;

	if (KM_FAILED(result = fs_deref_file_handle(file_handle, &file)))
		return result;

	return file->fs->ops.size(file, size_out);
}

void kn_file_destructor(om_object_t *obj) {
	mm_kfree(PB_CONTAINER_OF(fs_file_t, object_header, obj));
}

km_result_t fs_find_file(om_handle_t file_handle, fs_finddata_t *finddata, om_handle_t *file_handle_out) {
	fs_file_t *file;
	km_result_t result;

	if (KM_FAILED(result = fs_deref_file_handle(file_handle, &file)))
		return result;

	if (file->filetype != FS_FILETYPE_DIR)
		return KM_MAKEERROR(KM_RESULT_INVALID_ARGS);

	fs_dir_exdata_t *exdata = (fs_dir_exdata_t *)fs_file_exdata(file);
	kf_hashmap_node_t *node = kf_hashmap_begin(&exdata->children);
	finddata->node = node;

	if (node) {
		kn_child_file_entry_t *registry = PB_CONTAINER_OF(kn_child_file_entry_t, hashmap_header, node);
		*file_handle_out = registry->file_handle;
	} else {
		*file_handle_out = OM_INVALID_HANDLE;
	}

	return KM_RESULT_OK;
}

void fs_find_next_file(fs_finddata_t *finddata, om_handle_t *file_handle_out) {
	kf_hashmap_node_t *node = kf_hashmap_next(finddata->node);
	finddata->node = node;

	if (node) {
		kn_child_file_entry_t *registry = PB_CONTAINER_OF(kn_child_file_entry_t, hashmap_header, node);
		*file_handle_out = registry->file_handle;
	} else {
		*file_handle_out = OM_INVALID_HANDLE;
	}
}
