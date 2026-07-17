#include <hal/x86_64/irq.hh>
#include <stdalign.h>

PBOS_EXTERN_C_BEGIN

alignas(16) arch_gate_t hali_kidt[512] = {};

size_t kh_get_irq_max() {
	return PBOS_ARRAYSIZE(hali_kidt) / 2;
}

void hali_set_isr(hal_isr_t isr, size_t irq, uint8_t dpl, uint8_t gate_type) {
	kd_assert(irq <= PBOS_ARRAYSIZE(hali_kidt) / 2);
	kd_assert(dpl <= 3);
	hali_kidt[(irq * 2)] = GATEDESC_LOW(isr, SELECTOR_KCODE, GATEDESC_ATTRIBS(1, dpl, 0, gate_type));
	hali_kidt[(irq * 2) + 1] = GATEDESC_HIGH(isr);
}

void kh_set_isr(hal_isr_t isr, size_t irq) {
	if (irq > kh_get_irq_max())
		return;
	hali_set_isr(isr, irq, 0, GATE_INT386);
}

PBOS_EXTERN_C_END
