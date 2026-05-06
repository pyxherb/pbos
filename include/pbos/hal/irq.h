#ifndef _HAL_IRQ_H_
#define _HAL_IRQ_H_

#include <pbos/common.h>
#include <pbos/attribs.h>

PBOS_EXTERN_C_BEGIN

typedef struct _hal_irq_context_t hal_irq_context_t;

extern hal_irq_context_t **irq_contexts;

typedef PBOS_NORETURN void (*hal_isr_t)();

bool irq_is_disabled();
void irq_disable();
void irq_enable();

size_t hal_irq_getmax();
void kh_set_isr(hal_isr_t isr, size_t irq);
hal_isr_t hal_irq_getisr();

void kh_irq_init();

PBOS_EXTERN_C_END

#endif
