#ifndef _PBOS_KH_KD_STACKTRACE_H_
#define _PBOS_KH_KD_STACKTRACE_H_

#include <pbos/kd/stacktrace.h>

void *kh_stack_trace_begin();
void *kh_stack_trace_next(void *cur);
void *kh_stack_trace_get_address(void *frame);

#endif
