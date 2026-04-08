.global hn_load_user_context

hn_load_user_context:
	popq %rax // Eliminate extraneous previous RIP

	popq %rdi
	popq %rsi
	popq %rbp
	popq %rbx
	popq %rdx
	popq %rcx

	popq %rax
	movw %ax, %fs
	popq %rax
	movw %ax, %es
	popq %rax
	movw %ax, %gs
	popq %rax
	movw %ax, %ds

	popq %rax

	iretq

.global hn_load_kernel_context

hn_load_kernel_context:
	popq %rax // Eliminate extraneous previous EIP

	popq %rdi
	popq %rsi
	popq %rbp
	popq %rbx
	popq %rdx
	popq %rcx

	popq %rax
	movw %ax, %fs
	popq %rax
	movw %ax, %es
	popq %rax
	movw %ax, %gs
	popq %rax
	movw %ax, %ds

	popq %rax

	movl 8(%rax), %esp
	pushq 16(%rax) // EFLAGS
	pushq 24(%rax) // CS
	pushq 32(%rax) // EIP
	movl 0(%rax), %eax

	iretq
