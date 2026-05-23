#include <pbos/io/ctb.h>
#include <pbos/kfxx/allocator.hh>
#include <pbos/kfxx/scope_guard.hh>
#include <pbos/kf/misc.h>

PBOS_EXTERN_C_BEGIN

PBOS_API km_result_t io_alloc_ctb(io_ctb_ops_t *ops, size_t exdata_size, io_ctb_t **ctb_out) {
	size_t size = sizeof(io_ctb_t) + exdata_size;
	io_ctb_t *ctb = (io_ctb_t *)kfxx::kernel_allocator()->alloc(size, alignof(io_ctb_t));
	if(!ctb)
		return KM_RESULT_NO_MEM;

	kfxx::construct_at<io_ctb_t>(ctb);

	ctb->size = size;

	kfxx::scope_guard release_ctb_guard([ctb]() noexcept {
		io_destroy_ctb(ctb);
	});

	ctb->prev = nullptr;
	ctb->next = nullptr;

	memcpy(&ctb->ops, ops, sizeof(*ops));

	*ctb_out = ctb;

	release_ctb_guard.release();

	return KM_RESULT_OK;
}

PBOS_API void io_destroy_ctb(io_ctb_t *ctb) {
	size_t size = ctb->size;
	ctb->ops.destroy(ctb);
	kfxx::destroy_at<io_ctb_t>(ctb);
	kfxx::kernel_allocator()->release(ctb, size, alignof(io_ctb_t));
}

PBOS_API void *io_get_ctb_exdata(io_ctb_t *ctb) {
	return &ctb[1];
}

PBOS_EXTERN_C_END
