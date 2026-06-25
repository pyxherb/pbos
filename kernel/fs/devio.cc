#include <pbos/kf/hash.h>
#include <pbos/mm/mm.h>
#include <string.h>
#include <pbos/ki/fs/devio.hh>

PBOS_EXTERN_C_BEGIN

fs_filesys_ops_t ki_devio_ops = {
	.subnode = ki_devio_subnode,
	.offload = ki_devio_offload,
	.create_file = ki_devio_create_file,
	.open = ki_devio_open,
	.close = ki_devio_close,
	.seek = ki_devio_seek,
	.read = ki_devio_read,
	.write = ki_devio_write,
	.pread = ki_devio_pread,
	.pwrite = ki_devio_pwrite,
	.ioctl = ki_devio_ioctl,
	.size = ki_devio_size,
	.destroy = ki_devio_destroy,
	.premount = ki_devio_premount,
	.unmount_cleanup = ki_devio_unmount_cleanup,
	.destructor = ki_devio_destructor
};

fs_filesys_t *ki_devio_filesys = nullptr;

fs::fnode_ptr ki_devio_root_dir;

km_result_t ki_devio_subnode(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out) {
	return KM_RESULT_NOT_FOUND;
}

void ki_devio_offload(fs_fnode_t *file) {
}

km_result_t ki_devio_create_file(io_dispatch_context_t *dc, fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out) {
	// Deny to create normal files, only special device nodes can be created.
	return KM_RESULT_UNSUPPORTED_OPERATION;
}

km_result_t ki_devio_create_dir(io_dispatch_context_t *dc, fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out) {
	fs::fnode_write_lock_guard g(parent);

	fs::fnode_ptr fnode;
	KM_RETURN_IF_FAILED(fs_alloc_dir_fnode(ki_devio_filesys, &fnode));

	KM_RETURN_IF_FAILED(fs_link_subnode(parent, fnode.get()));

	*file_out = fnode.get();

	fs_ref_fnode(*file_out);

	return KM_RESULT_OK;
}

km_result_t ki_devio_remove_file(io_dispatch_context_t *dc, fs_fnode_t *fnode) {
	dm_device_t *device = ((ki_devio_file_exdata_t *)fs_get_fnode_exdata(fnode))->device;

	switch (fs_get_fnode_type(fnode)) {
		case FS_FNODE_TYPE_DIR: {
			ki_devio_dir_exdata_t *fnode_exdata = (ki_devio_dir_exdata_t *)fs_get_fnode_exdata(fnode);

			if(fnode_exdata->first_child)
				return KM_RESULT_DIR_NOT_EMPTY;
			break;
		}
		case FS_FNODE_TYPE_FILE:
			KM_RETURN_IF_FAILED(device->ops.remove(dc, device));
			break;
		default:
			return KM_RESULT_UNSUPPORTED_OPERATION;
	}

	fs_fnode_t *parent = fs_parent_of(fnode);

	// Cannot delete the root devio directory.
	if (fs_filesys_of_fnode(parent) != ki_devio_filesys)
		return KM_RESULT_INVALID_ARGS;

	{
		ki_devio_fnode_exdata_t *base_exdata = (ki_devio_fnode_exdata_t *)fs_get_fnode_exdata(parent);
		ki_devio_dir_exdata_t *parent_exdata = (ki_devio_dir_exdata_t *)fs_get_fnode_exdata(parent);

		if (base_exdata->prev) {
			ki_devio_fnode_exdata_t *prev_exdata = (ki_devio_fnode_exdata_t *)fs_get_fnode_exdata(base_exdata->prev);
			prev_exdata->next = base_exdata->next;
		}

		if (base_exdata->next) {
			ki_devio_fnode_exdata_t *next_exdata = (ki_devio_fnode_exdata_t *)fs_get_fnode_exdata(base_exdata->next);
			next_exdata->prev = base_exdata->prev;
		}

		if (parent_exdata->first_child == fnode) {
			parent_exdata->first_child = base_exdata->next;
		}
	}

	dm_unref_device(device);

	KM_RETURN_IF_FAILED(fs_unlink_subnode(fnode));

	// TODO: Delete the fnode?

	return KM_RESULT_OK;
}

km_result_t ki_devio_open(fs_fnode_t *file, fs_fcb_t **fcb_out, fs_open_flags_t flags) {
	dm_device_t *device = ((ki_devio_file_exdata_t *)fs_get_fnode_exdata(file))->device;
	return device->ops.open(device, fcb_out, flags);
}

void ki_devio_close(fs_fcb_t *fcb) {
	dm_device_t *device = ((ki_devio_file_exdata_t *)fs_get_fnode_exdata(fs_get_file_of_fcb(fcb)))->device;
	device->ops.close(fcb);
}

km_result_t ki_devio_seek(io_dispatch_context_t *dc, fs_fcb_t *fcb, long off, fs_seek_mode_t mode) {
	return ((ki_devio_file_exdata_t *)fs_get_fnode_exdata(fs_get_file_of_fcb(fcb)))->device->ops.seek(dc, fcb, off, mode);
}

km_result_t ki_devio_read(io_dispatch_context_t *dc, fs_fcb_t *fcb, char *dest, size_t size, size_t *bytes_read_out) {
	return ((ki_devio_file_exdata_t *)fs_get_fnode_exdata(fs_get_file_of_fcb(fcb)))->device->ops.read(dc, fcb, dest, size, bytes_read_out);
}

km_result_t ki_devio_write(io_dispatch_context_t *dc, fs_fcb_t *fcb, const void *src, size_t size, size_t *bytes_written_out) {
	return ((ki_devio_file_exdata_t *)fs_get_fnode_exdata(fs_get_file_of_fcb(fcb)))->device->ops.write(dc, fcb, src, size, bytes_written_out);
}

km_result_t ki_devio_pread(io_dispatch_context_t *dc, fs_fcb_t *fcb, char *dest, size_t size, size_t off, size_t *bytes_read_out) {
	return ((ki_devio_file_exdata_t *)fs_get_fnode_exdata(fs_get_file_of_fcb(fcb)))->device->ops.pread(dc, fcb, dest, size, off, bytes_read_out);
}

km_result_t ki_devio_pwrite(io_dispatch_context_t *dc, fs_fcb_t *fcb, const void *src, size_t size, size_t off, size_t *bytes_written_out) {
	return ((ki_devio_file_exdata_t *)fs_get_fnode_exdata(fs_get_file_of_fcb(fcb)))->device->ops.pwrite(dc, fcb, src, size, off, bytes_written_out);
}

km_result_t ki_devio_ioctl(io_dispatch_context_t *dc, fs_fcb_t *fcb, uint32_t ioctl_code, void *data_in, size_t size_in, void *data_out, size_t size_out, void *args) {
	return ((ki_devio_file_exdata_t *)fs_get_fnode_exdata(fs_get_file_of_fcb(fcb)))->device->ops.ioctl(dc, fcb, ioctl_code, data_in, size_in, data_out, size_out, args);
}

km_result_t ki_devio_size(io_dispatch_context_t *dc, fs_fcb_t *fcb, size_t *size_out) {
	return ((ki_devio_file_exdata_t *)fs_get_fnode_exdata(fs_get_file_of_fcb(fcb)))->device->ops.size(dc, fcb, size_out);
}

km_result_t ki_devio_enum_first_child_file(fs_fnode_t *dir, fs_fnode_t **first_file_out) {
	fs::fnode_read_lock_guard g(dir);
	if (fs_get_fnode_type(dir) != FS_FNODE_TYPE_DIR)
		return KM_RESULT_UNSUPPORTED_OPERATION;
	*first_file_out = ((ki_devio_dir_exdata_t *)fs_get_fnode_exdata(dir))->first_child;
	fs_ref_fnode(*first_file_out);
	return KM_RESULT_OK;
}

km_result_t ki_devio_enum_next_file(fs_fnode_t *cur_file, fs_fnode_t **next_file_out) {
	if (!cur_file) {
		*next_file_out = nullptr;
		return KM_RESULT_INVALID_ARGS;
	}

	fs::fnode_read_lock_guard g(cur_file);
	if (fs_get_fnode_type(cur_file) != FS_FNODE_TYPE_FILE)
		return KM_RESULT_UNSUPPORTED_OPERATION;
	fs_unref_fnode(cur_file);
	if ((*next_file_out = ((ki_devio_fnode_exdata_t *)fs_get_fnode_exdata(cur_file))->next))
		fs_ref_fnode(*next_file_out);
	return KM_RESULT_OK;
}

void ki_devio_destroy(fs_fnode_t *file) {
	{
		ki_devio_fnode_exdata_t *base_exdata = (ki_devio_fnode_exdata_t *)fs_get_fnode_exdata(file);
		fs::fnode_ptr parent(fs_parent_of(file));
		if (parent && (fs_filesys_of_fnode(parent.get()) == ki_devio_filesys)) {
			ki_devio_dir_exdata_t *parent_exdata = (ki_devio_dir_exdata_t *)fs_get_fnode_exdata(file);

			if (base_exdata->prev) {
				ki_devio_fnode_exdata_t *prev_exdata = (ki_devio_fnode_exdata_t *)fs_get_fnode_exdata(base_exdata->prev);
				prev_exdata->next = base_exdata->next;
			}

			if (base_exdata->next) {
				ki_devio_fnode_exdata_t *next_exdata = (ki_devio_fnode_exdata_t *)fs_get_fnode_exdata(base_exdata->next);
				next_exdata->prev = base_exdata->prev;
			}

			if (parent_exdata->first_child == file) {
				parent_exdata->first_child = base_exdata->next;
			}
		}
	}

	switch (fs_get_fnode_type(file)) {
		case FS_FNODE_TYPE_FILE: {
			ki_devio_file_exdata_t *exdata = (ki_devio_file_exdata_t *)fs_get_fnode_exdata(file);
			kfxx::destroy_and_release<ki_devio_file_exdata_t>(kfxx::kernel_allocator(), exdata);
			break;
		}
		case FS_FNODE_TYPE_DIR: {
			ki_devio_dir_exdata_t *exdata = (ki_devio_dir_exdata_t *)fs_get_fnode_exdata(file);
			kfxx::destroy_and_release<ki_devio_dir_exdata_t>(kfxx::kernel_allocator(), exdata);
			break;
		}
	}
}

km_result_t ki_devio_premount(fs_fnode_t *parent, fs_fnode_t *file) {
	return KM_RESULT_OK;
}

km_result_t ki_devio_unmount_cleanup(fs_fnode_t *file) {
	return KM_RESULT_OK;
}

km_result_t ki_devio_destructor() {
	return KM_RESULT_UNSUPPORTED_OPERATION;
}

void ki_devio_init() {
	km_result_t result;

	if (!(ki_devio_filesys = fs_register_file_system("initcar", strlen("initcar"), &ki_devio_ops, nullptr)))
		km_panic("Error registering devio file system");

	fs::fnode_ptr mount_point;
	{
		if (KM_FAILED(result = fs_create_child_dir(fs_abs_root_dir, KI_DEVIO_ROOT_DIR_NAME.data(), KI_DEVIO_ROOT_DIR_NAME.size(), &mount_point)))
			km_panic("Error creating devio mount point, error code = %.0x", result);
	}

	if (KM_FAILED(result = fs_alloc_dir_fnode(ki_devio_filesys, ki_devio_root_dir.get_addr_without_release())))
		km_panic("Error creating devio root directory, error code = %.0x", result);

	ki_devio_dir_exdata_t *exdata = (ki_devio_dir_exdata_t *)mm_kalloc(sizeof(ki_devio_dir_exdata_t), alignof(ki_devio_dir_exdata_t));

	if (!exdata)
		km_panic("Error allocating exdata for devio root directory");

	fs_set_fnode_exdata(ki_devio_root_dir.get(), exdata);

	if (KM_FAILED(result = fs_mount(mount_point.get(), ki_devio_root_dir.get())))
		km_panic("Error mounting devio root directory, error code = %x\n", result);
}

void ki_devio_deinit() {
	// stub
}

PBOS_EXTERN_C_END
