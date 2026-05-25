#include <pbos/ki/io/dispatch.hh>

PBOS_EXTERN_C_BEGIN

PBOS_API void io_init_dispatch_context(io_dispatch_context_t *dc) {
	dc->num_ctbs = 0;
	dc->first_ctb = nullptr;
	dc->last_ctb = nullptr;
}

PBOS_NODISCARD PBOS_API void io_push_ctb(io_dispatch_context_t *dc, io_ctb_t *ctb) {
	if (!dc->first_ctb)
		dc->first_ctb = ctb;
	if (dc->last_ctb)
		dc->last_ctb->next = ctb;
	ctb->prev = dc->last_ctb;
	dc->last_ctb = ctb;
	++dc->num_ctbs;
}

PBOS_API void io_pop_ctb(io_dispatch_context_t *dc) {
	io_ctb_t *ctb = dc->last_ctb;
	if (ctb->prev)
		ctb->prev->next = nullptr;
	if (ctb == dc->first_ctb)
		dc->first_ctb = nullptr;
	--dc->num_ctbs;
	ctb->ops.destroy(ctb);
}

PBOS_API io_ctb_t *io_current_ctb(io_dispatch_context_t *dc) {
	return dc->last_ctb;
}

PBOS_API size_t io_ctb_list_size(io_dispatch_context_t *dc) {
	return dc->num_ctbs;
}

PBOS_API bool io_is_ctb_list_empty(io_dispatch_context_t *dc) {
	return !dc->num_ctbs;
}

PBOS_API km_result_t ki_poll_ctbs(io_dispatch_context_t *dc) {
	km_result_t result;

	io_ctb_t *cur_ctb,
		*last_ctb = nullptr;
	while ((cur_ctb = io_current_ctb(dc)) != last_ctb) {
		if (KM_FAILED(result = cur_ctb->ops.perform(dc, cur_ctb))) {
			for (;;) {
				io_ctb_t *cur_ctb = io_current_ctb(dc);
				cur_ctb->ops.fail(dc, cur_ctb, result);
			}
		}
	}

	return KM_RESULT_OK;
}

PBOS_EXTERN_C_END
