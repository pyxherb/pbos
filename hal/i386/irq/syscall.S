.global isr_syscall
.align 4

//
// e[a-d]x: Arguments
//
isr_syscall:
	pushal

	pushw %ds

	pushw %es

	pushw %fs

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
	movw %ax, %fs
	movw %ax, %gs

	cld

	call hn_syscall_handler

	addl $24, %esp

	popw %ax
	movw %ax, %gs

	popw %ax
	movw %ax, %fs

	popw %ax
	movw %ax, %es

	popw %ax
	movw %ax, %ds

	popal
	iret
