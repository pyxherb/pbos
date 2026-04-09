.global isr_timer
.align 4
isr_timer:
	// Kernel mode: EFLAGS -> CS -> EIP
	// User mode: SS -> ESP -> EFLAGS -> CS -> EIP
	// xchg %bx, %bx

	pushq %rsp

	pushq %rbp
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %rbx
	pushq %rax

	movq %ds, %rax
	pushq %rax

	movq %es, %rax
	pushq %rax

	movq %fs, %rax
	pushq %rax

	movq %gs, %rax
	pushq %rax

	// xchg %bx, %bx

	call isr_timer_impl
