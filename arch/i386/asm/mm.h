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

#endif
