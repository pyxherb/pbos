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

	pushl %ds

	pushl %es

	pushl %gs

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

	sti

	call hn_syscall_handler

	cli

	addl $24, %esp

	movl %eax, %edx

	popl %eax
	movw %ax, %gs

	popl %eax
	movw %ax, %es

	popl %eax
	movw %ax, %ds

	movl %edx, %eax

	popl %edi
	popl %esi
	popl %ecx
	popl %ebx
	iretl
