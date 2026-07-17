#include <pbos/ki/io/iocb.hh>
#include <pbos/kfxx/allocator.hh>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/kf/misc.h>

PBOS_EXTERN_C_BEGIN

PBOS_API km_result_t io_alloc_iocb(io_iocb_ops_t *ops, size_t exdata_size, io_iocb_t **iocb_out) {
	size_t size = sizeof(io_iocb_t) + exdata_size;
	io_iocb_t *iocb = (io_iocb_t *)kfxx::kernel_allocator()->alloc(size, alignof(io_iocb_t));
	if(!iocb)
		return KM_RESULT_NO_MEM;

	kfxx::construct_at<io_iocb_t>(iocb);

	iocb->size = size;

	kfxx::scope_guard release_iocb_guard([iocb]() noexcept {
		io_destroy_iocb(iocb);
	});

	iocb->prev = nullptr;
	iocb->next = nullptr;

	memcpy(&iocb->ops, ops, sizeof(*ops));

	*iocb_out = iocb;

	release_iocb_guard.release();

	return KM_RESULT_OK;
}

PBOS_API void io_destroy_iocb(io_iocb_t *iocb) {
	size_t size = iocb->size;
	iocb->ops.destroy(iocb);
	kfxx::destroy_at<io_iocb_t>(iocb);
	kfxx::kernel_allocator()->release(iocb, size, alignof(io_iocb_t));
}

PBOS_API void *io_get_iocb_exdata(io_iocb_t *iocb) {
	return &iocb[1];
}

PBOS_EXTERN_C_END
