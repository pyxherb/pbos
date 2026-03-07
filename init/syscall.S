.global user_syscall
user_syscall:
	movl 4(%esp), %eax
	movl 8(%esp), %ebx
	movl 12(%esp), %ecx
	movl 16(%esp), %edx
	movl 20(%esp), %esi
	movl 24(%esp), %edi
	int $0xc0
	ret
