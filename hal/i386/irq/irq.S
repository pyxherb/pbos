.global isr_timer
isr_timer:
	// Kernel mode: EFLAGS -> CS -> EIP
	// User mode: SS -> ESP -> EFLAGS -> CS -> EIP
	// xchg %bx, %bx

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

	movl %esp, %eax
	addl $(14 * 4), %eax
	pushl %eax

	// xchg %bx, %bx

	call isr_timer_impl

	popl %eax

	popl %eax

	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp

	popl %eax
	movw %ax, %gs
	popl %eax
	movw %ax, %fs
	popl %eax
	movw %ax, %es
	popl %eax
	movw %ax, %ds

	iretl
