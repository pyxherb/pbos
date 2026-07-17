#include <pbos/kh/io/irq.hh>

PBOS_API bool io_is_irq_disabled() {
	return kh_is_irq_disabled();
}

PBOS_API void io_disable_irq() {
	return kh_disable_irq();
}

PBOS_API void io_enable_irq() {
	return kh_enable_irq();
}

PBOS_API size_t io_get_irq_max() {
	return kh_get_irq_max();
}
