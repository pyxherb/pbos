#ifndef _PBOS_KI_IO_IRQ_H_
#define _PBOS_KI_IO_IRQ_H_

#include <pbos/ki/io/context.hh>
#include <pbos/kfxx/bitarray.hh>

typedef struct _io_interrupt_t {
	io_interrupt_t *prev = nullptr, *next = nullptr;
	io_isr_t isr = nullptr;
	io_irq_id_t irq_id = 0;
	kfxx::bitarray_t enabled_masks;

	_io_interrupt_t(kfxx::allocator_t *allocator);
} io_interrupt_t;

#endif
