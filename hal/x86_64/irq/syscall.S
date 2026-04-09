.global isr_syscall
.align 4

//
// e[a-d]x: Arguments
//
isr_syscall:
	pushq %rbx
	pushq %rcx
	pushq %rsi
	pushq %rdi

	movq %ds, %rax
	pushq %rax

	movq %es, %rax
	pushq %rax

	movq %gs, %rax
	pushq %rax

	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rbx
	pushq %rax

	movw %ss, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %gs

	cld

	sti

	call hn_syscall_handler

	cli

	addq $24, %rsp

	movq %rax, %rdx

	popq %rax
	movw %ax, %gs

	popq %rax
	movw %ax, %es

	popq %rax
	movw %ax, %ds

	movq %rdx, %rax

	popq %rdi
	popq %rsi
	popq %rcx
	popq %rbx
	iretq
