#ifndef _PBOS_IO_IRQ_H_
#define _PBOS_IO_IRQ_H_

#include <pbos/km/result.h>

typedef struct _io_interrupt_t io_interrupt_t;

enum {
	IO_IRQ_RESULT_HANDLED = 0,
	IO_IRQ_RESULT_MISMATCHED,
};

typedef uint32_t io_irq_id_t;
typedef uint8_t io_irq_result_t;

typedef io_irq_result_t (*io_isr_t)(io_interrupt_t *interrupt, io_irq_id_t irq_id, const void *isr_frame);

PBOS_API bool io_is_local_irq_disabled();
PBOS_API void io_disable_local_irq();
PBOS_API void io_enable_local_irq();

PBOS_API size_t io_get_irq_max();

PBOS_API bool io_is_per_cpu_irq_supported();

enum {
	IO_REGISTER_IRQ_SHARED = 0,
};

typedef uint32_t io_register_irq_flags;

PBOS_API km_result_t io_register_irq(io_isr_t isr, size_t irq, io_register_irq_flags flags, io_interrupt_t **interrupt_out);
PBOS_API void io_free_irq(io_interrupt_t *interrupt);

#endif
