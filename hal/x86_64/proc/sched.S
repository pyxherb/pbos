.global hn_isr_timer
.align 8
hn_isr_timer:
	// Kernel mode: RFLAGS -> CS -> RIP
	// User mode: SS -> RSP -> RFLAGS -> CS -> RIP
	// xchg %bx, %bx

	pushq %rsp

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

	call hn_isr_timer_impl
