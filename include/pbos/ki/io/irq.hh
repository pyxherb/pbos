#ifndef _PBOS_KI_IO_IRQ_H_
#define _PBOS_KI_IO_IRQ_H_

#include <pbos/io/context.h>

typedef struct _io_interrupt_t {
	io_context_t *context = nullptr;
	io_interrupt_t *prev = nullptr, *next = nullptr;
	io_isr_t isr = nullptr;
	io_irq_id_t irq_id = 0;
} io_interrupt_t;

#endif
