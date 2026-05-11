MSG_STACKTRACE:
	.asciz "Stacktrace:\n"
MSG_STACKFRAME_TRACE:
	.asciz "\tFrame +%d: %p\n"

.global hal_stacktrace

hal_stacktrace:
pushl %rbp
movl %rsp, %rbp

	cld
		pushq $MSG_STACKTRACE
		call klog_printf
	addq $8, %rsp

	movq (%rsp), %rcx// ECX = Previous EBP
	xorq %rax, %rax// EAX = Number of current frame

	0:
		testl %rcx, %rcx
		jz 1f

		movq 8(%rcx), %rdx // EDX = Previous EIP
		movq (%rcx), %rcx // ECX = Previous EBP

		cld
			pushq %rdx
			pushq %rax
			pushq $MSG_STACKFRAME_TRACE
			call klog_printf
		add $12, %rsp

		incq %rax

	jmp 0b
	1:

leave
retl
