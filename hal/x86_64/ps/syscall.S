.global isr_syscall
.align 4

isr_syscall:
	pushq %rbx

	movq %ds, %rbx
	pushq %rbx

	movq %es, %rbx
	pushq %rbx

	movq %gs, %rbx
	pushq %rbx

	movw %ss, %bx
	movw %bx, %ds
	movw %bx, %es
	movw %bx, %gs

	cld

	sti

	call hn_syscall_handler

	cli

	movq %rax, %rdx

	popq %rbx
	movw %bx, %gs

	popq %rbx
	movw %bx, %es

	popq %rbx
	movw %bx, %ds

	movq %rdx, %rax

	popq %rbx
	iretq
