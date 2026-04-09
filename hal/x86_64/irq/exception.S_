MSG_UNHANDLED_KEXCEPT:
	.asciz "Unhandled kernel exception %d at %p:\n%s"
MSG_UNHANDLED_NAMED_EXCEPT:
	.asciz "Unhandled kernel exception %s at %p"
MSG_MACHINE_CHECK:
	.asciz "An uncorrectable hardware error was detected"

.macro panic_with_details ip
	cld
	pushl \ip
	pushl $1b
	pushl $MSG_UNHANDLED_NAMED_EXCEPT
	call km_panic
.endm

.global isr_diverr
1: .asciz "#DE"
isr_diverr:
	movl (%esp), %eax
	pushal

	cmpl $0x80000000, %eax
	jnl 0f

	popal
	iret
	0:
	panic_with_details %eax

.global isr_overflow
1: .asciz "#OF"
isr_overflow:
	movl (%esp), %eax
	pushal

	cmpl $0x80000000, %eax
	jnl 0f

	popal
	iret
	0:
	panic_with_details %eax

.global isr_boundrange
isr_boundrange:
	movl (%esp), %eax
	pushal

	cmpl $0x80000000, %eax
	jnl 0f

	popal
	iret
	0:
	panic_with_details %eax

.global isr_invl_opcode
isr_invl_opcode:
	movl (%esp), %eax
	pushal

	cmpl $0x80000000, %eax
	jnl 0f

	popal
	iret
	0:
	panic_with_details %eax

.global isr_nofpu
isr_nofpu:
	movl (%esp), %eax
	pushal

	cmpl $0x80000000, %eax
	jnl 0f

	popal
	iret
	0:
	panic_with_details %eax

.global isr_double_fault
isr_double_fault:
	movl (%esp), %eax
	pushal

	cmpl $0x80000000, %eax
	jnl 0f

	popal
	iret
	0:
	panic_with_details %eax
.global isr_dev_invltss
isr_dev_invltss:
	movl (%esp), %eax
	pushal

	cmpl $0x80000000, %eax
	jnl 0f

	popal
	iret
	0:
	panic_with_details %eax

.global isr_noseg
isr_noseg:
	movl (%esp), %eax
	pushal

	cmpl $0x80000000, %eax
	jnl 0f

	popal
	iret
	0:
	panic_with_details %eax

.global isr_stackerr
isr_stackerr:
	movl (%esp), %eax
	pushal

	cmpl $0x80000000, %eax
	jnl 0f

	popal
	iret
	0:
	panic_with_details %eax

.global isr_prot
isr_prot:
	movl (%esp), %eax
	pushal

	cmpl $0x80000000, %eax
	jnl 0f

	popal
	iret
	0:
	panic_with_details %eax

.global isr_pgfault
isr_pgfault:
	movl (%esp), %eax
	pushal

	cmpl $0x80000000, %eax
	jnl 0f

	popal
	iret
	0:
	panic_with_details %eax

.global isr_fpuerr
isr_fpuerr:
	movl (%esp), %eax
	pushal

	cmpl $0x80000000, %eax
	jnl 0f

	popal
	iret
	0:
	panic_with_details %eax

.global isr_alignchk
isr_alignchk:
	movl (%esp), %eax
	pushal

	cmpl $0x80000000, %eax
	jnl 0f

	popal
	iret
	0:
	panic_with_details %eax

.global isr_machchk
isr_machchk:
	cld
	pushl $MSG_MACHINE_CHECK
	call km_panic

.global isr_simderr
isr_simderr:
	movl (%esp), %eax
	pushal

	cmpl $0x80000000, %eax
	jnl 0f

	popal
	iret
	0:
	panic_with_details %eax

.global isr_virterr
isr_virterr:
	movl (%esp), %eax
	pushal

	cmpl $0x80000000, %eax
	jnl 0f

	popal
	iret
	0:
	panic_with_details %eax

.global isr_ctrlprot
isr_ctrlprot:
	movl (%esp), %eax
	pushal

	cmpl $0x80000000, %eax
	jnl 0f

	popal
	iret
	0:
	panic_with_details %eax

.global isr_hverr
isr_hverr:
	movl (%esp), %eax
	pushal

	cmpl $0x80000000, %eax
	jnl 0f

	popal
	iret
	0:
	panic_with_details %eax

.global isr_vmmerr
isr_vmmerr:
	movl (%esp), %eax
	pushal

	cmpl $0x80000000, %eax
	jnl 0f

	popal
	iret
	0:
	panic_with_details %eax

.global isr_securityerr
isr_securityerr:
	movl (%esp), %eax
	pushal

	cmpl $0x80000000, %eax
	jnl 0f

	popal
	iret
	0:
	panic_with_details %eax
