#ifndef _PBOS_FMT_ELF_H_
#define _PBOS_FMT_ELF_H_

#include <pbos/common.h>

/// @brief ELF machines.
enum {
	EM_NONE = 0,		  // No Machine
	EM_M32,				  // AT&T WE 32100
	EM_SPARC,			  // SPARC
	EM_386,				  // Intel 80386
	EM_68K,				  // Motorola 68000
	EM_88K,				  // Motorola 88000
	EM_IAMCU,			  // Intel MCU
	EM_860,				  // Intel 80860
	EM_MIPS,			  // MIPS I
	EM_S370,			  // IBM System/370
	EM_MIPS_RS3_LE,		  // MIPS RS3000 Little-endian
	EM_PARISC = 15,		  // HP PA-RISC
	EM_VPP500 = 17,		  // Fujitsu VPP500
	EM_SPARC32PLUS,		  // SPARC V8 Plus
	EM_960,				  // Intel 80960
	EM_PPC,				  // PowerPC
	EM_PPC64,			  // PowerPC 64-bit
	EM_S390,			  // IBM System/390
	EM_SPU,				  // IBM SPU/SPC
	EM_V800 = 36,		  // NEC V800
	EM_FR20,			  // Fujitsu FR20
	EM_RH32,			  // TRW RH-32
	EM_RCE,				  // Motorola RCE
	EM_ARM,				  // AArch32
	EM_ALPHA,			  // DEC Alpha
	EM_SH,				  // Hitachi SH
	EM_SPARCV9,			  // SPARC 9
	EM_TRICORE,			  // Siemens TriCore
	EM_ARC,				  // Argonaut RISC Core
	EM_H8_300,			  // Hitachi H8/300
	EM_H8_300H,			  // Hitachi H8/300H
	EM_H8S,				  // Hitachi H8S
	EM_H8_500,			  // Hitachi H8/500
	EM_IA_64,			  // IA-64
	EM_MIPS_X,			  // MIPS-X
	EM_COLDFIRE,		  // Motorola ColdFire
	EM_68HC12,			  // Motorola 68HC12
	EM_MMA,				  // Fujitsu MMA
	EM_PCP,				  // Siemens PCP
	EM_NCPU,			  // Sony nCPU
	EM_NDR1,			  // Denso NDR1
	EM_STARCORE,		  // Motorola StarCore
	EM_ME16,			  // ME16
	EM_ST100,			  // STMicroelectronics ST100
	EM_TINYJ,			  // Advanced Logic TinyJ
	EM_X86_64,			  // AMD x86-64
	EM_PDSP,			  // Sony DSP
	EM_PDP10,			  // DEC PDP-10
	EM_PDP11,			  // DEC PDP-11
	EM_FX66,			  // Siemens FX66
	EM_ST9PLUS,			  // STMicroelectronics ST9
	EM_ST7,				  // STMicroelectronics ST7
	EM_68HC16,			  // Motorola MC68HC16
	EM_68HC11,			  // Motorola MC68HC11
	EM_68HC08,			  // Motorola MC68HC08
	EM_68HC05,			  // Motorola MC68HC05
	EM_SVX,				  // SGI SVx
	EM_ST19,			  // STMicroelectronics ST19
	EM_VAX,				  // DEC VAX
	EM_CRIS,			  // CRIS Machines by Axis
	EM_JAVELIN,			  // Infineon Javelin
	EM_FIREPATH,		  // Element14 FirePath
	EM_ZSP,				  // LSI ZSP
	EM_MMIX,			  // 64-bit RISC Machine by Donald Knuth
	EM_HUANY,			  // Harvard University's Object Files
	EM_PRISM,			  // SiTera Prism
	EM_AVR,				  // Atmel AVR
	EM_FR30,			  // Fujitsu FR30
	EM_D10V,			  // Mitsubishi D10V
	EM_D30V,			  // Mitsubishi D30V
	EM_V850,			  // NEC v850
	EM_M32R,			  // Mitsubishi M32R
	EM_MN10300,			  // Panasonic MN10300
	EM_MN10200,			  // Panasonic MN10200
	EM_PJ,				  // PicoJava
	EM_OPENRISC,		  // OpenRISC 32-bit
	EM_ARC_COMPACT,		  // ARC Compact
	EM_XTENSA,			  // Xtensa
	EM_VIDEOCORE,		  // Alphamosaic VideoCore
	EM_TMM_GPP,			  // Thompson Multimedia General Purpose Processor
	EM_NS32K,			  // National Semiconductor 32000
	EM_TPC,				  // Tenor Network TPC
	EM_SNP1K,			  // Trebia SNP 1000
	EM_ST200,			  // STMicroelectronics ST200
	EM_IP2K,			  // Ubicom IP2000 Series
	EM_MAX,				  // PB_MAX Processor
	EM_CR,				  // Compact RISC
	EM_F2MC16,			  // Fujitsu F2MC16
	EM_MSP430,			  // Texas Instruments MSP430
	EM_BLACKFIN,		  // ADI BlackFin
	EM_SE_C33,			  // Epson S1C33
	EM_SEP,				  // Sharp Embedded Processor
	EM_ARCA,			  // Arca RISC Processor
	EM_UNICORE,			  // UniCore
	EM_EXCESS,			  // eXcess
	EM_DXP,				  // Icera DXP
	EM_ALTERA_NIOS2,	  // Altera Nios II
	EM_CRX,				  // CompactRISC CRX
	EM_XGATE,			  // Motorola XGATE
	EM_C166,			  // Infineon C16x/XC16x
	EM_M16C,			  // Renesas M16C
	EM_DSPIC30F,		  // Microchip DSPIC30F
	EM_CE,				  // Freescale Communication Engine
	EM_M32C,			  // Renesas M32C
	EM_TSK3000 = 131,	  // Altium TSK3000
	EM_RS08,			  // Freescale RS08
	EM_SHARC,			  // ADI SHARC
	EM_ECOG2,			  // Cyan Technology eCOG2
	EM_SCORE7,			  // Sunplus S+core7
	EM_DSP24,			  // NJR/Nisshinbo 24-bit DSP
	EM_VIDEOCORE3,		  // Broadcom VideoCore
	EM_LATTICEMICO32,	  // LatticeMico32
	EM_SE_C17,			  // EPSON C17
	EM_TI_C6000,		  // Texas Instruments TMS320C6000
	EM_TI_C2000,		  // Texas Instruments TMS320C2000
	EM_TI_C5500,		  // Texas Instruments TMS320C5500
	EM_TI_ARP32,		  // Texas Instruments Application Specific RISC Processor
	EM_TI_PRU,			  // Texas Instruments PRU
	EM_MMDSP_PLUS = 160,  // STMicroelectronics's 64-bit VLIW DSP
	EM_CYPRESS_M8C,		  // Cypress M8C
	EM_R32C,			  // Renesas R32C
	EM_TRIMEDIA,		  // NXP TriMedia
	EM_QDSP6,			  // Qualcomm DSP6
	EM_8501,			  // Intel 8501
	EM_STXP7X,			  // STMicroelectronics STxP7x
	EM_NDS32,			  // Andes NDS32
	EM_ECOG1,
	EM_ECOG1X = EM_ECOG1,  // Cyan Technology eCOG1x
	EM_MAXQ30,			   // Dallas MAXQ30
	EM_XIMO16,			   // NZ NJR/Nisshinbo 16-bit DSP
	EM_MANIK,			   // M2000 RISC Processor
	EM_CRAYNV2,			   // Cray NV2
	EM_RX,				   // Renesas RX
	EM_METAG,			   // Imagination META Processor
	EM_MCST_ELBRUS,		   // MCST Elbrus
	EM_ECOG16,			   // Cyan Technology eCOG16
	EM_CR16,			   // CompactRISC CR16
	EM_ETPU,			   // Freescale Extended Time Processing Unit
	EM_SLE9X,			   // Infineon SLE9X
	EM_L10M,			   // Intel L10M
	EM_K10M,			   // Intel K10M
	EM_AARCH64 = 183,	   // ARM AArch 64
	EM_AVR32 = 185,		   // Atmel AVR32
	EM_STM8,			   // STMicroelectronics STM8
	EM_TILE64,			   // Tilera TILE64
	EM_TILEPRO,			   // Tilera TILEPro
	EM_MICROBLAZE,		   // Xilinx MicroBlaze
	EM_CUDA,			   // NVIDIA CUDA
	EM_TILEGX,			   // Tilera TILE-Gx
	EM_CLOUDSHIELD,		   // CloudShield
	EM_COREA_1ST,		   // KIPO-KAIST Core-A 1st generation
	EM_COREA_2ND,		   // KIPO-KAIST Core-A 2nd generation
	EM_ARC_COMPACT2,	   // Synopsys ARCompact 2
	EM_OPEN8,			   // Open8
	EM_RL78,			   // Renesas RL78
	EM_VIDEOCORE5,		   // Broadcom VideoCore V
	EM_78KOR,			   // Renesas 78KOR
	EM_56800EX,			   // Freescale 56800EX DSC
	EM_BA1,				   // Beyond BA1
	EM_BA2,				   // Beyond BA2
	EM_XCORE,			   // XMOS xCORE
	EM_MCHP_PIC,		   // Microchip PIC
	EM_KM32 = 210,		   // KM211 KM32
	EM_KMX32,			   // KM211 KMX32
	EM_KMX16,			   // KM211 KMX16
	EM_KMX8,			   // KM211 KMX8
	EM_KVARC,			   // KM211 KVARC
	EM_CDP,				   // Paneve CDP
	EM_COGE,			   // Cognitive Smart Memory Processor
	EM_COOL,			   // Bluechip CoolEngine
	EM_NORC,			   // Nanoradio Optimized RISC
	EM_CSR_KALIMBA,		   // CSR Kalimba
	EM_Z80,				   // Zilog Z80
	EM_VISIUM,			   // VISIUMcore
	EM_FT32,			   // FTDI FT32
	EM_MOXIE,			   // Moxie Processors
	EM_AMDGPU,			   // AMD GPU
	EM_RISCV = 243,		   // RISC-V
};

typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Word;
typedef int32_t Elf32_Sword;

//
// ELF File Types
//
#define ET_NONE 0
#define ET_REL 1
#define ET_EXEC 2
#define ET_DYN 3
#define ET_CORE 4
#define ET_LOOS 0xfe00
#define ET_HIOS 0xfeff
#define ET_LOPROC 0xff00
#define ET_HIPROC 0xffff

//
// Indices for ELF Identifier
//
#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5
#define EI_VERSION 6
#define EI_OSABI 7
#define EI_ABIVERSION 8
#define EI_PAD 9
#define EI_NIDENT 16

//
// ELF Magic Numbers
//
#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

//
// ELF Classes
//
#define ELFCLASSNONE 0
#define ELFCLASS32 1
#define ELFCLASS64 2

//
// ELF Data Encodings
//
#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

//
// ELF Versions
//
#define EV_NONE 0
#define EV_CURRENT 1

//
// ELF ABIs
//
#define ELFOSABI_NONE 0
#define ELFOSABI_HPUX 1
#define ELFOSABI_NETBSD 2
#define ELFOSABI_GNU 3
#define ELFOSABI_LINUX 3
#define ELFOSABI_SOLARIS 6
#define ELFOSABI_AIX 7
#define ELFOSABI_IRIX 8
#define ELFOSABI_FREEBSD 9
#define ELFOSABI_TRU64 10
#define ELFOSABI_MODESTO 11
#define ELFOSABI_OPENBSD 12
#define ELFOSABI_OPENVMS 13
#define ELFOSABI_NSK 14
#define ELFOSABI_AROS 15
#define ELFOSABI_FENIXOS 16
#define ELFOSABI_CLOUDABI 17
#define ELFOSABI_OPENVOS 18

//
// ELF Header (EHDR)
//
typedef struct _Elf32_Ehdr {
	uint8_t e_ident[EI_NIDENT];	 // ELF Identification Information
	Elf32_Half e_type;			 // Object File Type
	Elf32_Half e_machine;		 // Target Machine
	Elf32_Word e_version;		 // ELF Version

	Elf32_Addr e_entry;	 // Entry

	Elf32_Off e_phoff;	// Program Header Offset
	Elf32_Off e_shoff;	// Section Header Offset

	Elf32_Word e_flags;	 // Flags

	Elf32_Half e_ehsize;  // ELF Header Size

	Elf32_Half e_phentsize;	 // Program Header Entry Size
	Elf32_Half e_phnum;		 // Program Header Number

	Elf32_Half e_shentsize;	 // Section Header Entry Size
	Elf32_Half e_shnum;		 // Section Header Number
	Elf32_Half e_shstrndx;	 // Index of Strings Section Header
} Elf32_Ehdr;

//
// Special Section Indexes
//
#define SHN_UNDEF 0x0000
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC 0xff1f
#define SHN_LOOS 0xff20
#define SHN_HIOS 0xff3f
#define SHN_ABS 0xfff1
#define SHN_COMMON 0xfff2
#define SHN_XINDEX 0xffff
#define SHN_HIRESERVE 0xffff

//
// Section Types
//
#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11
#define SHT_INIT_ARRAY 14
#define SHT_FINI_ARRAY 15
#define SHT_PREINIT_ARRAY 16
#define SHT_GROUP 17
#define SHT_SYMTAB_SHNDX 18
#define SHT_LOOS 0x600000000
#define SHT_HIOS 0x6fffffff
#define SHT_LOPROC 0x70000000
#define SHT_HIPROC 0x7fffffff
#define SHT_LOUSER 0x80000000
#define SHT_HIUSER 0xffffffff

//
// Section Attribute Flags
//
#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHF_MERGE 0x10
#define SHF_STRINGS 0x20
#define SHF_INFO_LINK 0x40
#define SHF_OS_NONCONFORMING 0x100
#define SHF_GROUP 0x200
#define SHF_TLS 0x400
#define SHF_COMPRESSED 0x800
#define SHF_MASKOS 0x0ff00000
#define SHF_MASKPROC 0xf0000000

//
// Section Groups
//
#define GRP_COMDAT 0x1
#define GRP_MASKOS 0x0ff00000
#define GRP_MASKPROC 0xf0000000

//
// Section Header (SHDR)
//
typedef struct _Elf32_Shdr {
	Elf32_Word sh_name;		  // Location of Section Name
	Elf32_Word sh_type;		  // Section Type
	Elf32_Word sh_flags;	  // Attribute Flags
	Elf32_Addr sh_addr;		  // Address
	Elf32_Off sh_offset;	  // Offset in File
	Elf32_Word sh_size;		  // Size
	Elf32_Word sh_link;		  // Table Index Link (Depends on the Section Type)
	Elf32_Word sh_info;		  // Extra Information
	Elf32_Word sh_addralign;  // Address alignment
	Elf32_Word sh_entsize;	  // Section Header Entry Size
} Elf32_Shdr;

//
// ELF Compression Types
//
#define ELFCOMPRESS_ZLIB 1
#define ELFCOMPRESS_LOOS 0x60000000
#define ELFCOMPRESS_HIOS 0x6fffffff
#define ELFCOMPRESS_LOPROC 0x70000000
#define ELFCOMPRESS_HIPROC 0x7fffffff

//
// Compression Header (CHDR)
//
typedef struct _Elf32_Chdr {
	Elf32_Word ch_type;		  // Algorithm Type
	Elf32_Word ch_size;		  // Uncompressed Data Size
	Elf32_Word ch_addralign;  // Alignment of Uncompressed Data
} Elf32_Chdr;

//
// Special Symbol Indexes
//
#define STN_UNDEF 0

//
// Symbol Bindings
//
#define STB_LOCAL 0
#define STB_GLOBAL 1
#define STB_WEAK 2
#define STB_LOOS 10
#define STB_HIOS 12
#define STB_LOPROC 13
#define STB_HIPROC 15

//
// Symbol Types
//
#define STT_NOTYPE 0
#define STT_OBJECT 1
#define STT_FUNC 2
#define STT_SECTION 3
#define STT_FILE 4
#define STT_COMMON 5
#define STT_TLS 6
#define STT_LOOS 10
#define STT_HIOS 12
#define STT_LOPROC 13
#define STT_HIPROC 15

//
// Symbol Visibilities
//
#define STV_DEFAULT 0
#define STV_INTERNAL 1
#define STV_HIDDEN 2
#define STV_PROTECTED 3

#define ELF32_ST_BIND(i) ((i) >> 4)
#define ELF32_ST_TYPE(i) ((i)&0xf)
#define ELF32_ST_INFO(b, t) (((b) << 4) + ((t)&0xf))

#define ELF32_ST_VISIBILITY(o) ((o)&0x3)

//
// Symbol Table Entry
//
typedef struct _Elf32_Sym {
	Elf32_Word st_name;		 // Symbol Name Index in Symbol String Table
	Elf32_Addr st_value;	 // Symbol Value
	Elf32_Word st_size;		 // Symbol Size
	unsigned char st_info;	 // Symbol Information
	unsigned char st_other;	 // Visibility (Currently)
	Elf32_Half st_shndx;	 // Index of Related Section
} Elf32_Sym;

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
#define PT_TLS 7
#define PT_LOOS 0x60000000
#define PT_HIOS 0x6fffffff
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7fffffff

#define PF_X 0x1
#define PF_W 0x2
#define PF_R 0x4
#define PF_MASKOS 0x0ff00000
#define PF_MASKPROC 0xf0000000

//
// Program Header (PHDR)
//
typedef struct _Elf32_Phdr {
	Elf32_Word p_type;	  // Program Segment Type
	Elf32_Off p_offset;	  // Offset in File
	Elf32_Addr p_vaddr;	  // Virtual Address to Load
	Elf32_Addr p_paddr;	  // Physical Address To Load
	Elf32_Word p_filesz;  // Segment Size in the File
	Elf32_Word p_memsz;	  // Segment Size in The Memory
	Elf32_Word p_flags;	  // Segment Flags
	Elf32_Word p_align;	  // Segment Alignment
} Elf32_Phdr;

#endif
