.global isr_timer
.align 4
isr_timer:
	// Kernel mode: EFLAGS -> CS -> EIP
	// User mode: SS -> ESP -> EFLAGS -> CS -> EIP
	// xchg %bx, %bx

	pushl %esp

	pushl %ds
	pushl %es
	pushl %fs
	pushl %gs

	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %eax

	// xchg %bx, %bx

	call isr_timer_impl
