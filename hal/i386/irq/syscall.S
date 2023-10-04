.global _irq_syscall_handler
.align 4

//
// e[a-d]x: Arguments
//
_irq_syscall_handler:
	pushal

	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %eax

	cld

	movl hn_syscall_entries(, %esi, 4), %eax
	call *(%eax, %edi, 4)

	addl $16, %esp

	popal
	iret
