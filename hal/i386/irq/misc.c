#include <hal/i386/irq.h>
#include <stdalign.h>

alignas(16) arch_gate_t hn_kidt[256] = { 0 };

size_t irq_getmax()
{
	return PB_ARRAYSIZE(hn_kidt);
}

void hn_setisr(hal_isr_t isr, size_t irq, uint8_t dpl, uint8_t gate_type) {
	assert(irq <= PB_ARRAYSIZE(hn_kidt));
	assert(dpl <= 3);
	hn_kidt[irq] = GATEDESC(isr, SELECTOR_KCODE, GATEDESC_ATTRIBS(1, dpl, 0, gate_type));
}

void irq_setisr(hal_isr_t isr, size_t irq)
{
	if (irq > irq_getmax())
		return;
	hn_kidt[irq] = GATEDESC(isr, SELECTOR_KCODE, GATEDESC_ATTRIBS(1, 0x00, 0, GATE_INT386));
}
