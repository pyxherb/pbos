#include <arch/i386/asm/mm.h>

.global hn_load_user_context

hn_load_user_context:
	popl %eax // Eliminate extraneous previous EIP

	popl %edi
	popl %esi
	popl %ebp
	popl %ebx
	popl %edx
	popl %ecx

	popl %eax
	movw %ax, %fs
	popl %eax
	movw %ax, %es
	popl %eax
	movw %ax, %gs
	popl %eax
	movw %ax, %ds

	popl %eax

	iretl

.global hn_load_kernel_context

hn_load_kernel_context:
	popl %eax // Eliminate extraneous previous EIP

	popl %edi
	popl %esi
	popl %ebp
	popl %ebx
	popl %edx
	popl %ecx

	popl %eax
	movw %ax, %fs
	popl %eax
	movw %ax, %es
	popl %eax
	movw %ax, %gs
	popl %eax
	movw %ax, %ds

	popl %eax

	movl 4(%eax), %esp
	pushl 8(%eax) // EFLAGS
	pushl 16(%eax) // CS
	pushl 12(%eax) // EIP
	movl 0(%eax), %eax

	iretl
