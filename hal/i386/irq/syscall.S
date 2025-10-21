.global isr_syscall
.align 4

//
// e[a-d]x: Arguments
//
isr_syscall:
	pushl %ebx
	pushl %ecx
	pushl %esi
	pushl %edi

	pushw %ds

	pushw %es

	pushw %gs

	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %eax

	movw %ss, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %gs

	cld

	call hn_syscall_handler

	cli

	addl $24, %esp

	movl %eax, %edx

	popw %ax
	movw %ax, %gs

	popw %ax
	movw %ax, %es

	popw %ax
	movw %ax, %ds

	movl %edx, %eax

	popl %edi
	popl %esi
	popl %ecx
	popl %ebx
	iret
