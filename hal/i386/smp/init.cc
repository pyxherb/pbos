#include <arch/i386/apic.h>
#include <pbos/kn/km/smp.h>
#include <pbos/km/logger.h>
#include "../irq.h"

PBOS_EXTERN_C_BEGIN

uint32_t hn_sched_interval;

void smp_init() {
	smp_main_eu_init();
}

void hn_calibrate_apic() {
	uint8_t lapic_intvec = arch_read_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_SPURIOUS_INT_VEC) & 0xff;

	arch_cli();

	// Enable LAPIC
	arch_write_lapic(
		hn_lapic_vbase,
		ARCH_LAPIC_REG_SPURIOUS_INT_VEC,
		lapic_intvec | ARCH_LAPIC_SPURIOUS_INT_VEC_REG_ENABLE);

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_DIVIDE_CONFIG, ARCH_LAPIC_DIVIDE_CONFIG_16);

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_LVT_TIMER, ARCH_LAPIC_LVT_TIMER_REG_ONESHOT);

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_EOI, 0);

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_INITIAL_COUNT, 0xffffffff);

	// Delay for 1000 microseconds to calibrate the APIC.
	arch_out8(0x43, 0x30);

	constexpr uint16_t COUNT_RATE = 11931;
	arch_out8(0x40, COUNT_RATE & 0xff);
	arch_out8(0x40, (COUNT_RATE >> 8) & 0xff);

	while (true) {
		arch_out8(0x43, 0x00);
		uint16_t cur_count = arch_in8(0x40);
		cur_count |= arch_in8(0x40) << 8;

		if (!cur_count)
			break;
	}

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_LVT_TIMER, ARCH_LAPIC_LVT_TIMER_REG_MASKED);
	uint32_t cur_timer = arch_read_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_CURRENT_COUNT);
	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_INITIAL_COUNT, 0);
	hn_sched_interval = (0xffffffff - cur_timer) / 10;
	// hn_sched_interval = 1234;

	arch_write_lapic(hn_lapic_vbase, ARCH_LAPIC_REG_EOI, 0);

	arch_sti();
}

void smp_main_eu_init() {
	arch_cli();

	hal_irq_context_t *ctxt = hal_irq_contexts[0];

	hn_calibrate_apic();

	klog_printf("Timer interval ticks: %u\n", hn_sched_interval);
}

PBOS_EXTERN_C_END
