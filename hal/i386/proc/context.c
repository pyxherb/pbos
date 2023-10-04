#include <arch/i386/reg.h>
#include <hal/i386/proc.h>

void hn_save_context(hn_proc_context_t *ctxt) {
	// Save general-purpose registers.
	{
		__asm__ volatile("movl %esp, %eax");  // Save ESP

		__asm__ volatile("movl %0, %%esp"
						 : "=m"(*(((char *)&ctxt->gp_regs) + (sizeof(ctxt->gp_regs) - 1))));
		__asm__ volatile("pushal");

		__asm__ volatile("movl %eax, %esp");  // Restore ESP
	}

	ctxt->eip = arch_reip();

	__asm__ volatile("pushfl");
	__asm__ volatile("popl %0"
					 : "=m"(ctxt->eflags));

	__asm__ volatile("movw %%cs, %0"
					 : "=r"(ctxt->cs));
	__asm__ volatile("movw %%ds, %0"
					 : "=r"(ctxt->ds));
	__asm__ volatile("movw %%ss, %0"
					 : "=r"(ctxt->ss));
	__asm__ volatile("movw %%es, %0"
					 : "=r"(ctxt->es));
	__asm__ volatile("movw %%fs, %0"
					 : "=r"(ctxt->fs));
	__asm__ volatile("movw %%gs, %0"
					 : "=r"(ctxt->gs));

	__asm__ volatile("movl %%cr0, %0"
					 : "=r"(ctxt->cr0));
	__asm__ volatile("movl %%cr2, %0"
					 : "=r"(ctxt->cr2));
	__asm__ volatile("movl %%cr4, %0"
					 : "=r"(ctxt->cr4));

	if (arch_rcr4() & CR4_OSXSAVE) {
		// TODO:Save XCR register.
		__asm__ volatile("xorl %ecx, %ecx");
		__asm__ volatile("xgetbv"
						 : "=a"(ctxt->xcr0));
	}
}
