#ifndef _ARCH_I386_ASM_MM_H_
#define _ARCH_I386_ASM_MM_H_

#define KERNEL_PBASE 0x02000000
#define KERNEL_VBASE 0x80000000
#define KERNEL_MAX_SIZE 0x1000000

#define GDTDESC(base, limit, access_byte, flags)\
	.short (limit & 0x0ffff);\
	.short (base & 0x0000ffff);\
	.byte (base & 0x00ff0000 >> 16);\
	.byte (access_byte);\
	.byte (limit & 0xf0000 >> 16) | (flags << 4);\
	.byte (base & 0xff000000 >> 24)

#define GDT_ACCESS_BYTE(pr, dpl, s, ex, dc, rw, accessed) (accessed | rw << 1 | dc << 2 | ex << 3 | s << 4 | dpl << 5 | pr << 7)
#define GDT_FLAGS(gr, sz) (sz << 2 | gr << 3)
#define SELECTOR(rpl, ti, index) (rpl | ti << 2 | index << 3)

#define PADDR_TOP(prefix) ((prefix##_PBASE) + ((prefix##_SIZE) - 1))
#define VADDR_TOP(prefix) ((prefix##_VBASE) + ((prefix##_SIZE) - 1))

//
// Kernel Bottom Area
//
#define KBOTTOM_PBASE 0x00000000
#define KBOTTOM_VBASE 0x00000000
#define KBOTTOM_SIZE 0x00400000
#define KBOTTOM_PTOP PADDR_TOP(KBOTTOM)
#define KBOTTOM_VTOP VADDR_TOP(KBOTTOM)

//
// User Image Area
//
#define UIMAGE_VBASE 0x00400000
#define UIMAGE_SIZE (0x30000000 - KBOTTOM_SIZE)
#define UIMAGE_VTOP VADDR_TOP(UIMAGE)

//
// User Border
//
#define UBORDER_VBASE 0x7ffff000
#define UBORDER_SIZE 0x00001000
#define UBORDER_VTOP VADDR_TOP(UBORDER)

//
// User Free Area
// Can be used as heap, stack, etc.
//
#define UFREE_VBASE 0x30000000
#define UFREE_SIZE (0x50000000 - UBORDER_SIZE)
#define UFREE_VTOP VADDR_TOP(UFREE)

//
// Kernel Image
//
#define KERNEL_PBASE 0x02000000
#define KERNEL_VBASE 0x80000000
#define KERNEL_SIZE 0x00c00000
#define KERNEL_PTOP (KERNEL_PBASE + KERNEL_SIZE - 1)
#define KERNEL_VTOP (KERNEL_VBASE + KERNEL_SIZE - 1)

//
// Kernel PDT
//
#define KPDT_PBASE (KERNEL_PTOP + 1)
#define KPDT_VBASE (KERNEL_VTOP + 1)
#define KPDT_SIZE 0x00001000
#define KPDT_PTOP (KPDT_PBASE + KPDT_SIZE - 1)
#define KPDT_VTOP (KPDT_VBASE + KPDT_SIZE - 1)

//
// Kernel Page Tables
//
#define KPGT_PBASE (KPDT_PTOP + 1)
#define KPGT_VBASE (KPDT_VTOP + 1)
#define KPGT_SIZE 0x00004000  // Can map the whole of critical area
#define KPGT_PTOP (KPGT_PBASE + KPGT_SIZE - 1)
#define KPGT_VTOP (KPGT_VBASE + KPGT_SIZE - 1)

//
// Kernel Page Table for `0x00000000` - `0x003fffff`
//
#define KBOTTOMPGT_PBASE (KPGT_PTOP + 1)
#define KBOTTOMPGT_VBASE (KPGT_VTOP + 1)
#define KBOTTOMPGT_SIZE 0x00001000
#define KBOTTOMPGT_PTOP (KBOTTOMPGT_PBASE + KBOTTOMPGT_SIZE - 1)
#define KBOTTOMPGT_VTOP (KBOTTOMPGT_VBASE + KBOTTOMPGT_SIZE - 1)

//
// Kernel Stack
//
#define KSTACK_PBASE (KBOTTOMPGT_PTOP + 1)
#define KSTACK_VBASE (KBOTTOMPGT_VTOP + 1)
#define KSTACK_SIZE 0x0000a000
#define KSTACK_PTOP (KSTACK_PBASE + KSTACK_SIZE - 1)
#define KSTACK_VTOP (KSTACK_VBASE + KSTACK_SIZE - 1)

//
// Initializable Part of Critical Area
//
#define INIT_CRITICAL_PBASE CRITICAL_PBASE
#define INIT_CRITICAL_VBASE CRITICAL_VBASE
#define INIT_CRITICAL_SIZE \
	(KERNEL_SIZE + KPDT_SIZE + KPGT_SIZE + KBOTTOMPGT_SIZE + KSTACK_SIZE)
#define INIT_CRITICAL_PTOP \
	(INIT_CRITICAL_PBASE + INIT_CRITICAL_SIZE - 1)
#define INIT_CRITICAL_VTOP \
	(INIT_CRITICAL_VBASE + INIT_CRITICAL_SIZE - 1)

//
// Kernel Temporary Mapping Area
//
#define KTMPMAP_VBASE (INIT_CRITICAL_VTOP + 1)
#define KTMPMAP_SIZE 0x00100000
#define KTMPMAP_VTOP (KTMPMAP_VBASE + KTMPMAP_SIZE - 1)

//
// Kernel Top-level Page Directory
//
#define KTOPPGTAB_VBASE (KTMPMAP_VTOP + 1)
#define KTOPPGTAB_SIZE 0x00200000
#define KTOPPGTAB_VTOP (KTOPPGTAB_VBASE + KTOPPGTAB_SIZE - 1)

//
// Context switching temporary data
//
#define KCTXTSWTMP_VBASE (KTOPPGTAB_VTOP + 1)
#define KCTXTSWTMP_SIZE 0x10000
#define KCTXTSWTMP_VTOP VADDR_TOP(KCTXTSWTMP)

//
// Critical Area
//
#define CRITICAL_PBASE KERNEL_PBASE
#define CRITICAL_VBASE KERNEL_VBASE
#define CRITICAL_SIZE ((INIT_CRITICAL_SIZE) + (KTMPMAP_SIZE) + (KTOPPGTAB_SIZE) + (KCTXTSWTMP_SIZE))
#define CRITICAL_PTOP (CRITICAL_PBASE + CRITICAL_SIZE - 1)
#define CRITICAL_VTOP (CRITICAL_VBASE + CRITICAL_SIZE - 1)

#define KRESERVED_VBASE (CRITICAL_VTOP + 1)
#define KRESERVED_SIZE (CRITICAL_SIZE)
#define KRESERVED_VTOP (KRESERVED_VBASE + KRESERVED_SIZE)

#define USPACE_VBASE 0x00400000
#define USPACE_VTOP 0x7fffffff
#define USPACE_SIZE (USPACE_VTOP - USPACE_VBASE + 1)

#define KSPACE_VBASE 0x80000000
#define KSPACE_VTOP 0xffffffff
#define KSPACE_SIZE (KSPACE_VTOP - KSPACE_VBASE + 1)

#endif
