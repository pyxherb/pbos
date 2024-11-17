MSG_STACKTRACE:
	.asciz "Stacktrace:\n"
MSG_STACKFRAME_TRACE:
	.asciz "\tFrame +%d: %p\n"

.global hal_stacktrace

hal_stacktrace:
pushl %ebp
movl %esp, %ebp

	cld
		pushl $MSG_STACKTRACE
		call kprintf
	addl $4, %esp

	movl (%esp), %ecx// ECX = Previous EBP
	xorl %eax, %eax// EAX = Number of current frame

	0:
		testl %ecx, %ecx
		jz 1f

		movl 4(%ecx), %edx // EDX = Previous EIP
		movl (%ecx), %ecx // ECX = Previous EBP

		cld
			pushl %edx
			pushl %eax
			pushl $MSG_STACKFRAME_TRACE
			call kprintf
		add $12, %esp

		incl %eax

	jmp 0b
	1:

leave
retl
