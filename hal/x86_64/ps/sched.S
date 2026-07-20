.global hali_timer_isr
.align 8
hali_timer_isr:
	// User mode: SS -> RSP -> RFLAGS -> CS -> RIP
	// xchg %bx, %bx

	pushq %rbp
	pushq %rbx
	pushq %rax
	pushq %r15
	pushq %r14
	pushq %r13
	pushq %r12
	pushq %r11
	pushq %r10

	movq %ds, %rax
	pushq %rax

	movq %es, %rax
	pushq %rax

	movq %fs, %rax
	pushq %rax

	movq %gs, %rax
	pushq %rax

	// xchg %bx, %bx

	call hali_timer_isr_impl
