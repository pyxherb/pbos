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

km_result_t ki_devio_subnode(fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out) {
	return KM_RESULT_NOT_FOUND;
}

void ki_devio_offload(fs_fnode_t *file) {
}

km_result_t ki_devio_create_file(io_dispatch_context_t *dc, fs_fnode_t *parent, const char *name, size_t name_len, fs_fnode_t **file_out) {
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

km_result_t ki_devio_open(fs_fnode_t *file, fs_fcb_t **fcb_out) {
	dm_device_t *device = ((ki_devio_file_exdata_t *)fs_get_fnode_exdata(file))->device;
	return device->ops.open(device, fcb_out);
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

void ki_devio_destroy(fs_fnode_t *file) {
	ki_devio_file_exdata_t *exdata = (ki_devio_file_exdata_t *)fs_get_fnode_exdata(file);
	kfxx::destroy_and_release<ki_devio_file_exdata_t>(kfxx::kernel_allocator(), exdata);
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

PBOS_EXTERN_C_END
