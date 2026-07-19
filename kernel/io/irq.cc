#include <pbos/kh/io/irq.hh>
#include <pbos/ki/io/context.hh>

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

PBOS_API bool io_is_per_cpu_irq_supported() {
	return kh_is_per_cpu_irq_supported();
}

PBOS_API km_result_t io_register_irq(io_isr_t isr, size_t irq, io_interrupt_t **interrupt_out) {
	// TODO: Implement it.
}

PBOS_API km_result_t io_register_shared_irq(io_isr_t isr, size_t irq, io_interrupt_t **interrupt_out) {
	// TODO: Implement it.
}

PBOS_API void io_free_irq(io_interrupt_t *interrupt) {
	if (interrupt->prev)
		interrupt->prev->next = interrupt->next;
	if (interrupt->next)
		interrupt->next->prev = interrupt->prev;
	if(interrupt->context) {
		auto &interrupt_list = interrupt->context->registered_isrs.at(interrupt->irq_id);

		if(interrupt_list.first == interrupt) {
			if(!(interrupt_list.first = interrupt->next))
				interrupt->context->registered_isrs.remove(interrupt->irq_id);
		}
	}

	kfxx::destroy_and_release<io_interrupt_t>(kfxx::kernel_allocator(), interrupt);
}
