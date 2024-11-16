.global memset
memset:
pushl %ebp
movl %esp, %ebp
	movl 8(%ebp), %edi // dest
	movzbl 12(%ebp), %eax // c
	movl 16(%ebp), %ecx // n

	cld

	testl %ecx, 0b11
	jz 32f
	testl %ecx, 0b1
	jz 16f
		rep stosb
		jmp 0f
	32:
		shrl $2, %ecx
		rep stosl
		jmp 0f
	16:
		shrl $1, %ecx
		rep stosw
	0:
	movl 8(%esp), %eax // return value
leave
retl

.global memcmp
memcmp:
pushl %ebp
movl %esp, %ebp
	movl 8(%ebp), %edi // dest
	movl 12(%ebp), %esi // src
	movl 16(%ebp), %ecx // n

	cld

	testl %ecx, 0b11
	jz 32f
	testl %ecx, 0b1
	jz 16f

	//
	// 8-bit comparison
	//
		repe cmpsb

		// Jump if equal
		cmp $0, %ecx
		jz 0f

		movb (%esi), %al
		cmpb (%edi), %al
		ja 1f
		jb 2f

	//
	// 32-bit comparison
	//
	32:
		shrl $2, %ecx
		repe cmpsl

		// Jump if equal
		cmp $0, %ecx
		jz 0f

		shll $1, %ecx
	//
	// 16-bit comparison
	//
	16:
		shrl $1, %ecx
		repe cmpsw

		// Jump if equal
		cmp $0, %ecx
		jz 0f

		// Copy the last bytes for comparing
		movw (%esi), %ax
		movw (%edi), %dx

		// First byte
		cmpb %ah, %dh
		ja 1f // Greater
		jb 2f // Lesser

		// Second byte
		cmpb %al, %dl
		ja 1f // Greater
		jmp 2f // Lesser

	0: // Equal
	xorl %eax, %eax
	jmp 99f

	1: // Greater than
	movl %eax, 1
	jmp 99f

	2: // Less than
	movl %eax, -1
99: // End
leave
retl
