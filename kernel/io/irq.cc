#include <pbos/kh/io/irq.hh>
#include <pbos/ki/io/context.hh>
#include <pbos/ki/mp/misc.hh>

PBOS_API bool io_is_local_irq_disabled() {
	return kh_is_irq_disabled();
}

PBOS_API void io_disable_local_irq() {
	return kh_disable_irq();
}

PBOS_API void io_enable_local_irq() {
	return kh_enable_irq();
}

PBOS_API size_t io_get_irq_max() {
	return kh_get_irq_max();
}

PBOS_API bool io_is_per_cpu_irq_supported() {
	return kh_is_per_cpu_irq_supported();
}

kfxx::option_t<kfxx::radix_map_t<io_irq_id_t, io_interrupt_list_t>> ki_registered_isrs;

ps::semaphore_t ki_isr_registry_semaphore;

_io_interrupt_t::_io_interrupt_t(kfxx::allocator_t *allocator) : enabled_masks(allocator) {
}

PBOS_API km_result_t io_register_irq(io_isr_t isr, size_t irq, io_register_irq_flags flags, io_interrupt_t **interrupt_out) {
	ps::write_semaphore_guard g(ki_isr_registry_semaphore);

	if (!(flags & IO_REGISTER_IRQ_SHARED)) {
		if (ki_registered_isrs->contains(irq))
			return KM_RESULT_EXISTED;
	}

	io_interrupt_t *interrupt = kfxx::alloc_and_construct<io_interrupt_t>(kfxx::kernel_allocator(), kfxx::kernel_allocator());

	if (!interrupt)
		return KM_RESULT_NO_MEM;

	kfxx::scope_guard sg([interrupt]() noexcept {
		kfxx::destroy_and_release<io_interrupt_t>(kfxx::kernel_allocator(), interrupt);
	});

	if(!interrupt->enabled_masks.resize_uninit(mp_num_total_cpu)) {
		return KM_RESULT_NO_MEM;
	}

	io_interrupt_t **il;
	if (auto it = ki_registered_isrs->find(irq);
		it != ki_registered_isrs->end()) {
		il = &it->first;
	} else {
		if (!ki_registered_isrs->insert(irq, _io_interrupt_list_t()))
			return KM_RESULT_NO_MEM;
		il = &ki_registered_isrs->at(irq).first;
	}

	if (*il) {
		interrupt->next = *il;
		(*il)->prev = interrupt;
	}
	*il = interrupt;

	sg.release();

	return KM_RESULT_OK;
}

PBOS_API void io_free_irq(io_interrupt_t *interrupt) {
	ps::write_semaphore_guard g(ki_isr_registry_semaphore);
	if (interrupt->prev)
		interrupt->prev->next = interrupt->next;
	if (interrupt->next)
		interrupt->next->prev = interrupt->prev;
	auto &interrupt_list = ki_registered_isrs->at(interrupt->irq_id);

	if (interrupt_list.first == interrupt) {
		if (!(interrupt_list.first = interrupt->next))
			ki_registered_isrs->remove(interrupt->irq_id);
	}

	kfxx::destroy_and_release<io_interrupt_t>(kfxx::kernel_allocator(), interrupt);
}
