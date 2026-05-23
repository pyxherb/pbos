#include <pbos/kh/kd/stacktrace.h>
#include <cstddef>

struct hali_stack_frame_t {
	hali_stack_frame_t *rbp;
	void *rip;
};

void *kh_stack_trace_begin() {
	hali_stack_frame_t *stack_frame;
	asm volatile("movq %%rbp, %0" : "=r"(stack_frame));
	// Usually (Actual origin) -> km_stack_trace_begin -> kh_stack_trace_begin
	return stack_frame->rbp->rbp;
}

void *kh_stack_trace_next(void *cur) {
	hali_stack_frame_t *stack_frame = (hali_stack_frame_t *)cur;
	return stack_frame->rbp ? stack_frame->rbp : nullptr;
}

void *kh_stack_trace_get_address(void *frame) {
	return ((hali_stack_frame_t *)frame)->rip;
}
