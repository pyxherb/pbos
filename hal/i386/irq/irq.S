.global isr_timer
isr_timer:
	push %ebp
	push %edi
	push %esi
	push %edx
	push %ecx
	push %ebx
	push %eax

	cli

	call isr_timer_impl
