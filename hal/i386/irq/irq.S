.global isr_irq0
isr_irq0:
	push %ebp
	push %edi
	push %esi
	push %edx
	push %ecx
	push %ebx
	push %eax

	cli

	call isr_irq0_impl
