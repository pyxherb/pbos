.global isr_irq0
isr_irq0:
	cli

	call isr_irq0_impl

	iret
