.global user_syscall
user_syscall:
	pushq %r9
	pushq %r8
	int $0xc0
	ret
