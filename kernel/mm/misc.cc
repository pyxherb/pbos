#include <pbos/km/mm.h>
#include <pbos/km/proc.hh>

mm_context_t *mm_get_cur_context() {
	return mm_cur_contexts[ps_get_cur_euid()];
}
