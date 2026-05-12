#include <pbos/hal/irq.h>
#include <pbos/kd/logger.h>
#include <pbos/kd/stacktrace.h>
#include <pbos/kh/km/panic.h>
#include <stdarg.h>

PBOS_EXTERN_C_BEGIN

bool ki_is_panicked = false;

bool km_is_panicked() {
	return ki_is_panicked;
}

PBOS_NORETURN void km_panic(const char *fmt, ...) {
	irq_disable();
	if (km_is_panicked())
		goto panicked;

	ki_is_panicked = true;

	va_list args;
	va_start(args, fmt);
	klog_printf("\n>> PANIC <<\n");
	klog_vprintf(fmt, args);
	va_end(args);

	klog_printf("\nStack trace:\n");
	for (void *i = kd_stack_trace_begin(); i; i = kd_stack_trace_next(i)) {
		klog_printf("%p\n", i);
	}
panicked:
	kh_panic_halt();
}

PBOS_EXTERN_C_END
