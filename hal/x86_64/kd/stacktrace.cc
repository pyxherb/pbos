#include <pbos/kh/kd/stacktrace.h>
#include <cstddef>

struct hn_stack_frame_t {
	hn_stack_frame_t *rbp;
	void *rip;
};

void *kh_stack_trace_begin() {
	hn_stack_frame_t *stack_frame;
	asm volatile("movq %%rbp, %0" : "=r"(stack_frame));
	// Usually (Actual origin) -> km_stack_trace_begin -> kh_stack_trace_begin
	return stack_frame->rbp->rbp;
}

void *kh_stack_trace_next(void *cur) {
	hn_stack_frame_t *stack_frame = (hn_stack_frame_t *)cur;
	return stack_frame->rbp ? stack_frame->rbp : nullptr;
}

void *kh_stack_trace_get_address(void *frame) {
	return ((hn_stack_frame_t *)frame)->rip;
}
