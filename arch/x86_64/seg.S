.global arch_loadcs

arch_loadcs:
	popq %rax
	pushq %rdi

	pushq %rax

	lretq
