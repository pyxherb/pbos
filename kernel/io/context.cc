#include <pbos/ki/io/context.hh>

io_context_t **ki_per_cpu_io_contexts = nullptr;

_io_context_t::_io_context_t() {
}

PBOS_API io_context_t *io_get_cur_context() {
	return ki_per_cpu_io_contexts[ps_get_cur_cpuid()];
}
