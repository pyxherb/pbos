.global isr_timer
isr_timer:
	cli

	push %ebp
	push %edi
	push %esi
	push %edx
	push %ecx
	push %ebx
	push %eax

	call isr_timer_impl
