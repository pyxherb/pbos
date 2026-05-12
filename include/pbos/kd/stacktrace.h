#ifndef _PBOS_KD_STACKTRACE_H_
#define _PBOS_KD_STACKTRACE_H_

void *kd_stack_trace_begin();
void *kd_stack_trace_next(void *cur);
void *kd_stack_trace_get_address(void *frame);

#endif
