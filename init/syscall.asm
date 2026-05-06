[BITS 64]
global user_syscall
user_syscall:
	int 0xc0
	ret
