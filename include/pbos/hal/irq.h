#ifndef _HAL_IRQ_H_
#define _HAL_IRQ_H_

#include <pbos/common.h>
#include <pbos/attribs.h>

PBOS_EXTERN_C_BEGIN

typedef struct _hal_irq_context_t hal_irq_context_t;

extern hal_irq_context_t **hal_irq_contexts;

typedef PBOS_NORETURN void (*hal_isr_t)();

bool hal_is_irq_disabled();
void hal_disable_irq();
void hal_enable_irq();

size_t hal_irq_getmax();
void hal_irq_setisr(hal_isr_t isr, size_t irq);
hal_isr_t hal_irq_getisr();

void hal_irq_init();

PBOS_EXTERN_C_END

#endif
