#ifndef _HAL_IRQ_H_
#define _HAL_IRQ_H_

#include <oicos/common.h>
#include <oicos/attribs.h>

typedef void __noreturn (*hal_isr_t)();

size_t irq_getmax();
void irq_setisr(hal_isr_t isr, size_t irq);
hal_isr_t irq_getisr();

void irq_init();

#endif
