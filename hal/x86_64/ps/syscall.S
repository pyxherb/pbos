.global isr_syscall
.align 4

isr_syscall:
	pushq %rcx

	movq %ds, %rbx
	pushq %rcx

	movq %es, %rbx
	pushq %rcx

	movq %gs, %rbx
	pushq %rcx

	movw %ss, %bx
	movw %bx, %ds
	movw %bx, %es
	movw %bx, %gs

	cld

	call hali_pbcore_dispatcher

	movq %rax, %rdx

	popq %rcx
	movw %bx, %gs

	popq %rcx
	movw %bx, %es

	popq %rcx
	movw %bx, %ds

	movq %rdx, %rax

	popq %rcx
	iretq
