#include <pbos/io/irq.hh>
#include <pbos/kasan/utils.h>
#include <pbos/kd/logger.h>
#include <pbos/kd/stacktrace.h>
#include <pbos/kh/km/panic.h>
#include <stdarg.h>
#include <pbos/ki/ps/kmod.hh>
#include <kernel/generated/config.hh>

PBOS_EXTERN_C_BEGIN

bool ki_is_panicked = false;

bool km_is_panicked() {
	return ki_is_panicked;
}

PBOS_NO_ASAN PBOS_NORETURN PBOS_API void km_panic(const char *fmt, ...) {
	io_disable_local_irq();
#if KI_ENABLE_KASAN
	kasan_disable();
#endif
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
		void *ptr = kd_stack_trace_get_address(i);
		const char *func_name = "???";

		auto sym_node = ki_registered_kernel_symbol_query_tree.find_max_lteq(ptr);
		if (sym_node) {
			if (((char *)sym_node->rb_value) + static_cast<ki_kernel_symbol_t *>(sym_node)->len >= (char *)ptr)
				func_name = static_cast<ki_kernel_symbol_t *>(sym_node)->name;
		}
		klog_printf("%p<%s>\n", i, func_name);
	}
panicked:
	kh_panic_halt();
}

PBOS_EXTERN_C_END
