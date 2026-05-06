.global hn_load_user_context

hn_load_user_context:
	// Eliminate extraneous return address
	popq %rax

	xorq %rdx, %rdx

	// Write FS.
	popq %rax
	andq $0x000000000000ffff, %rax
	movq $0x00000000c0000100, %rcx
	wrmsr

	// Write GS.
	popq %rax
	andq $0x000000000000ffff, %rax
	movq $0x00000000c0000101, %rcx
	wrmsr

	popq %rdx
	popq %rcx

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
	// Eliminate extraneous return address
	popq %rax

	xorq %rdx, %rdx

	// Write FS.
	popq %rax
	andq $0x000000000000ffff, %rax
	movq $0x00000000c0000100, %rcx
	wrmsr

	// Write GS.
	popq %rax
	andq $0x000000000000ffff, %rax
	movq $0x00000000c0000101, %rcx
	wrmsr

	popq %rdx
	popq %rcx

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

	pushq 8(%rax) // RIP
	pushq 16(%rax) // CS
	pushq 24(%rax) // RFLAGS
	pushq 32(%rax) // RSP
	pushq 40(%rax) // SS
	movq 0(%rax), %rax

	iretq
