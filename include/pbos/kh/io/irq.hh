#ifndef _PBOS_KH_IO_IRQ_H_
#define _PBOS_KH_IO_IRQ_H_

#include <pbos/io/irq.h>

PBOS_EXTERN_C_BEGIN

typedef void (*kh_isr_t)();

bool kh_is_irq_disabled();
void kh_disable_irq();
void kh_enable_irq();

size_t kh_get_irq_max();
void kh_set_isr(kh_isr_t isr, size_t irq);
kh_isr_t kh_irq_get_isr();

void kh_irq_init();

PBOS_EXTERN_C_END

#endif
