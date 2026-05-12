#include <pbos/kh/kd/stacktrace.h>

void *kd_stack_trace_begin() {
	return kh_stack_trace_begin();
}

void *kd_stack_trace_next(void *cur) {
	return kh_stack_trace_next(cur);
}

void *kd_stack_trace_get_address(void *frame) {
	return kh_stack_trace_get_address(frame);
}
