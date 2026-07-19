#include <pbos/ki/io/context.hh>

_io_context_t::_io_context_t() {
}

PBOS_API io_context_t *io_get_cur_context() {
	return ki_per_cpu_io_contexts[ps_get_cur_cpuid()];
}
