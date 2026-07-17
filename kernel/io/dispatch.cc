#include <pbos/ki/io/dispatch.hh>

PBOS_EXTERN_C_BEGIN

PBOS_API void io_init_dispatch_context(io_dispatch_context_t *dc) {
	dc->num_iocbs = 0;
	dc->first_iocb = nullptr;
	dc->last_iocb = nullptr;
}

PBOS_NODISCARD PBOS_API void io_push_iocb(io_dispatch_context_t *dc, io_iocb_t *iocb) {
	if (!dc->first_iocb)
		dc->first_iocb = iocb;
	if (dc->last_iocb)
		dc->last_iocb->next = iocb;
	iocb->prev = dc->last_iocb;
	dc->last_iocb = iocb;
	++dc->num_iocbs;
}

PBOS_API void io_pop_iocb(io_dispatch_context_t *dc) {
	io_iocb_t *iocb = dc->last_iocb;
	if (iocb->prev)
		iocb->prev->next = nullptr;
	if (iocb == dc->first_iocb)
		dc->first_iocb = nullptr;
	--dc->num_iocbs;
	iocb->ops.destroy(iocb);
}

PBOS_API io_iocb_t *io_current_iocb(io_dispatch_context_t *dc) {
	return dc->last_iocb;
}

PBOS_API size_t io_iocb_list_size(io_dispatch_context_t *dc) {
	return dc->num_iocbs;
}

PBOS_API bool io_is_iocb_list_empty(io_dispatch_context_t *dc) {
	return !dc->num_iocbs;
}

PBOS_API km_result_t ki_poll_iocbs(io_dispatch_context_t *dc) {
	km_result_t result;

	io_iocb_t *cur_iocb,
		*last_iocb = nullptr;
	while ((cur_iocb = io_current_iocb(dc)) != last_iocb) {
		if (KM_FAILED(result = cur_iocb->ops.perform(dc, cur_iocb))) {
			for (;;) {
				io_iocb_t *cur_iocb = io_current_iocb(dc);
				cur_iocb->ops.fail(dc, cur_iocb, result);
			}
		}
	}

	return KM_RESULT_OK;
}

PBOS_EXTERN_C_END
