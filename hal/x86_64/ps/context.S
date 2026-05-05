.global hn_load_user_context

hn_load_user_context:
	pushq %rdx
	pushq %rcx

	xorq %rdx, %rdx

	// Write FS.
	popq %rax
	xorq $0x000000000000ffff, %rax
	movl $0xc0000100, %ecx
	wrmsr

	// Write GS.
	popq %rax
	xorq $0x000000000000ffff, %rax
	movl $0xc0000101, %ecx
	wrmsr

	popq %rcx
	popq %rdx

	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %r11
	popq %r10
	popq %rbp
	popq %rbx

	popq %rax
	movw %ax, %es
	popq %rax
	movw %ax, %ds

	popq %rax

	iretq

.global hn_load_kernel_context

hn_load_kernel_context:
	pushq %rdx
	pushq %rcx

	xorq %rdx, %rdx

	// Write FS.
	popq %rax
	xorq $0x000000000000ffff, %rax
	movl $0xc0000100, %ecx
	wrmsr

	// Write GS.
	popq %rax
	xorq $0x000000000000ffff, %rax
	movl $0xc0000101, %ecx
	wrmsr

	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %r11
	popq %r10
	popq %rbp
	popq %rbx

	popq %rax
	movw %ax, %es
	popq %rax
	movw %ax, %ds

	popq %rax

	movl 8(%rax), %esp
	pushq 16(%rax) // EFLAGS
	pushq 24(%rax) // CS
	pushq 32(%rax) // EIP
	movl 0(%rax), %eax

	iretq
