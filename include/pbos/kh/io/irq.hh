#ifndef _PBOS_KH_IO_IRQ_H_
#define _PBOS_KH_IO_IRQ_H_

#include <pbos/ki/io/irq.hh>

PBOS_EXTERN_C_BEGIN

bool kh_is_irq_disabled();
void kh_disable_irq();
void kh_enable_irq();

size_t kh_get_irq_max();

bool kh_is_per_cpu_irq_supported();

void kh_irq_init();

PBOS_EXTERN_C_END

#endif
