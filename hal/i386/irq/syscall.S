.global isr_syscall
.align 4

//
// e[a-d]x: Arguments
//
isr_syscall:
	pushal
	
	movw %ds, %ax
	pushw %ax
	
	movw %es, %ax
	pushw %ax
	
	movw %fs, %ax
	pushw %ax
	
	movw %gs, %ax
	pushw %ax

	movw %ss, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %eax

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
