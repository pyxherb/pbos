#include "devio.h"
#include <pbos/kf/misc.h>

PBOS_EXTERN_C_BEGIN

km_result_t kbdcls_devio_open(dm_device_t *device, fs_fnode_t *fnode, fs_fcb_t **fcb_out, fs_open_flags_t flags) {
	fs_fcb_t *fcb;

	KM_RETURN_IF_FAILED(fs_create_fcb(fnode, &fcb));

	fs_set_fcb_exdata(fcb, device);

	dm_ref_device(device);
}

void kbdcls_devio_close_cleanup(fs_fcb_t *fcb) {
	dm_device_t *dev = static_cast<dm_device_t *>(fs_get_fcb_exdata(fcb));
	if (dev)
		dm_unref_device(dev);
}

km_result_t kbdcls_devio_remove(io_dispatch_context_t *dc, dm_device_t *device, fs_fnode_t *fnode) {
	return KM_RESULT_UNSUPPORTED_OPERATION;
}

km_result_t kbdcls_devio_read(io_dispatch_context_t *dc, fs_fcb_t *fcb, void *dest, size_t size, size_t *bytes_read_out) {
	if ((!size) || (size % sizeof(kbd_event_in_t)))
		return KM_RESULT_INVALID_ARGS;

	size_t count = size / sizeof(kbd_event_in_t);
	auto device = static_cast<dm_device_t *>(fs_get_fcb_exdata(fcb));
	*bytes_read_out = 0;

	for (size_t i = 0; i < count; ++i) {
		kbd_event_in_t ev;

		KM_RETURN_IF_FAILED(static_cast<kbdcls_device_t *>(dm_get_device_exdata(device))->ops.receive_event(dc, device, &ev));

		memcpy(static_cast<char*>(dest) + *bytes_read_out, &ev, sizeof(ev));

		*bytes_read_out += sizeof(kbd_event_in_t);
	}

	return KM_RESULT_OK;
}
km_result_t kbdcls_devio_write(io_dispatch_context_t *dc, fs_fcb_t *fcb, const void *src, size_t size, size_t *bytes_written_out) {
	if ((!size) || (size % sizeof(kbd_event_out_t)))
		return KM_RESULT_INVALID_ARGS;

	size_t count = size / sizeof(kbd_event_out_t);
	auto device = static_cast<dm_device_t *>(fs_get_fcb_exdata(fcb));
	*bytes_written_out = 0;

	for (size_t i = 0; i < count; ++i) {
		kbd_event_out_t ev;

		memcpy(&ev, static_cast<const char*>(src) + *bytes_written_out, sizeof(ev));

		KM_RETURN_IF_FAILED(static_cast<kbdcls_device_t *>(dm_get_device_exdata(device))->ops.send_event(dc, device, &ev));

		*bytes_written_out += sizeof(kbd_event_out_t);
	}

	return KM_RESULT_OK;
}

km_result_t kbdcls_devio_pread(io_dispatch_context_t *dc, fs_fcb_t *fcb, void *dest, size_t size, size_t off, size_t *bytes_read_out) {
	return kbdcls_devio_read(dc, fcb, dest, size, bytes_read_out);
}
km_result_t kbdcls_devio_pwrite(io_dispatch_context_t *dc, fs_fcb_t *fcb, const void *src, size_t size, size_t off, size_t *bytes_written_out) {
	return kbdcls_devio_write(dc, fcb, src, size, bytes_written_out);
}

km_result_t kbdcls_devio_ioctl(io_dispatch_context_t *dc, fs_fcb_t *fcb, uint32_t ioctl_code, void *data_in, size_t size_in, void *data_out, size_t size_out, void *args) {
	return KM_RESULT_UNSUPPORTED_OPERATION;
}

PBOS_EXTERN_C_END
