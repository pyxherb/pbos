#include <arch/i386/asm/mm.h>

.global hn_load_user_context

hn_load_user_context:
	movl (KCTXTSWTMP_VBASE + 0), %edi
	movl (KCTXTSWTMP_VBASE + 4), %esi
	movl (KCTXTSWTMP_VBASE + 8), %ebp
	movl (KCTXTSWTMP_VBASE + 16), %ebx
	movl (KCTXTSWTMP_VBASE + 20), %edx
	movl (KCTXTSWTMP_VBASE + 24), %ecx

	movw (KCTXTSWTMP_VBASE + 4096 + 2), %ax
	mov %ax, %ds

	movw (KCTXTSWTMP_VBASE + 4096 + 4), %ax
	mov %ax, %es

	movw (KCTXTSWTMP_VBASE + 4096 + 6), %ax
	mov %ax, %fs

	movw (KCTXTSWTMP_VBASE + 4096 + 8), %ax
	mov %ax, %gs

	// Data selector for the stack
	movzwl %ax, %eax
	pushl %eax

	// ESP
	movl (KCTXTSWTMP_VBASE + 12), %eax
	pushl %eax

	// EFLAGS
	movl (KCTXTSWTMP_VBASE + 36), %eax
	pushl %eax

	// Code selector
	movzwl (KCTXTSWTMP_VBASE + 4096), %eax
	pushl %eax

	// EIP
	movl (KCTXTSWTMP_VBASE + 32), %eax
	pushl %eax

	movl (KCTXTSWTMP_VBASE + 28), %eax

	iretl
