#ifndef MB_H
#define MB_H

#include <pbos/attribs.h>
#include <pbos/common.h>
#include <pbos/pm/acpi.h>
#include <arch/i386/mlayout.h>

#define INITCAR_MODULE_NAME "initcar"

#define MB_BOOTLDR_MAGIC 0x36d76289

typedef struct PB_PACKED _mb_info_hdr_t {
	uint32_t sz_total;
	uint32_t reserved;
} mb_info_hdr_t;

//
// All types of boot information tags.
//
#define MB_TAG_END 0
#define MB_TAG_BOOT_CMDLINE 1
#define MB_TAG_loader_name 2
#define MB_TAG_MODULES 3
#define MB_TAG_BASIC_MEMINFO 4
#define MB_TAG_BIOS_BOOTDEV 5
#define MB_TAG_MEMMAP 6
#define MB_TAG_VBEINFO 7
#define MB_TAG_FRAMEBUFFER_INFO 8
#define MB_TAG_ELF_SYMBOLS 9
#define MB_TAG_APM_TABLE 10
#define MB_TAG_EFI32_SYSTABLE_PTR 11
#define MB_TAG_EFI64_SYSTABLE_PTR 12
#define MB_TAG_SMBIOS_TABLES 13
#define MB_TAG_OLD_ACPI_RSDP 14
#define MB_TAG_NEW_ACPI_RSDP 15
#define MB_TAG_NETINFO 16
#define MB_TAG_EFI_MEMMAP 17
#define MB_TAG_EFI_BOOTSRVC_TERMINATED_INDICATOR 18
#define MB_TAG_EFI32_IMAGE_HANDLER_PTR 19
#define MB_TAG_EFI64_IMAGE_HANDLER_PTR 20
#define MB_TAG_IMAGE_LOADBASE_PADDR 21

#define MB_ARCH_MEM_AVAILABLE 1
#define MB_ARCH_MEM_ACPI_INFO 3
#define MB_ARCH_MEM_HIBERNATION 4
#define MB_MEMTYPE_DEFECTIVE 5

//
// The header data before every boot information tag's body.
// The size should be header size plus body size.
//
typedef struct PB_PACKED _mbtaghdr_t {
	uint32_t type;
	uint32_t size;
} mbtaghdr_t;

//
// Type-specified boot information tag body data.
//

//
// Type = 1
// Boot Command Line
//
typedef struct PB_PACKED _mbtag_cmdline_t {
	// First character of the null-terminated string.
	char string;
} mbtag_cmdline_t;

//
// Type = 2
// Boot Loader Name
//
typedef struct PB_PACKED _mbtag_loader_name_t {
	// First character of the null-terminated string.
	char string;
} mbtag_loader_name_t;

//
// Type = 3
// Modules
//
typedef struct PB_PACKED _mbtag_mod_t {
	uint32_t mod_start;
	uint32_t mod_end;
	// First character of the null-terminated string.
	char string;
} mbtag_mod_t;

//
// Type = 4
// Basic Memory Information
//
typedef struct PB_PACKED _mbtag_basicmem_info_t {
	uint32_t lower;
	uint32_t upper;
} mbtag_basicmem_info_t;

//
// Type = 5
// BIOS Boot Device
//
typedef struct PB_PACKED _mbtag_bootdev_t {
	uint32_t bios_dev;
	uint32_t part;
	uint32_t subpart;
} mbtag_bootdev_t;

//
// Type = 6
// Memory Map
// !: There are [sz_entry] entries after this structure.
//
typedef struct PB_PACKED _mbtag_mmap_t {
	uint32_t sz_entry;
	uint32_t ver_entry;
} mbtag_mmap_t;

// Memory Map Entry
typedef struct PB_PACKED _mb_mmap_entry_t {
	uint64_t base;
	uint64_t length;
	uint32_t type;
	uint32_t reserved;
} mb_mmap_entry_t;

//
// Type = 7
// VBE Information
//
typedef struct PB_PACKED _mbtag_vbeinfo_t {
	uint16_t mode;
	uint16_t interface_seg;
	uint16_t interface_off;
	uint16_t interface_len;
	uint8_t ctrl_info[512];
	uint8_t mode_info[256];
} mbtag_vbeinfo_t;

//
// Type = 8
// Framebuffer Information
// ! There's color information data after this structure and the data type is decided by the framebuffer type.
//
typedef struct PB_PACKED _mbtag_framebuf_info_t {
	uint64_t addr;
	uint32_t pitch;
	uint32_t width;
	uint32_t height;
	uint8_t bpp;
	uint8_t type;
	uint8_t reserved;
} mbtag_framebuf_info_t;

// Framebuffer Type = 0
// ! There are [framebuffer_palette_num_colors] framebuffer palettes after this structure.
typedef struct PB_PACKED _mb_framebuf_colorinfo_hdr_t {
	uint32_t color_num;
} mb_framebuf_colorinfo_hdr_t;

// Framebuffer palettes after the header.
typedef struct PB_PACKED _mb_framebuf_pal_hdr_t {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} mb_framebuf_pal_hdr_t;

// Framebuffer Type = 1
typedef struct PB_PACKED _mb_framebuf_direct_rgb_colorinfo_t {
	uint8_t rfield_pos;
	uint8_t rmask_size;
	uint8_t gfield_pos;
	uint8_t gmask_size;
	uint8_t bfield_pos;
	uint8_t bmask_size;
} mb_framebuf_direct_rgb_colorinfo_t;

//
// Type = 9
// ELF Symbols
//
typedef struct PB_PACKED _mbtag_elf_symbols_t {
	uint32_t size;
	uint16_t num;
	uint16_t entsize;
	uint16_t shndx;
	uint16_t reserved;
} mbtag_elf_symbols_t;

//
// Type = 10
// APM Table
//
typedef struct PB_PACKED _mbtag_apmtab_t {
	uint16_t version;
	uint16_t cseg;
	uint32_t offset;
	uint16_t cseg16;
	uint16_t dseg;
	uint16_t flags;
	uint16_t cseg_len;
	uint16_t cseg_len16;
	uint16_t dseg_len;
} mbtag_apmtab_t;

//
// Type = 11
// EFI 32-bit System Table Pointer
//
typedef struct PB_PACKED _mbtag_efi_systab32_t {
	uint32_t ptr;
} MultibootTagEFISysTable32;

//
// Type = 12
// EFI 64-bit System Table Pointer
//
typedef struct PB_PACKED _mbtag_efi_systab64_t {
	uint64_t ptr;
} mbtag_efi_systab64_t;

//
// Type = 13
// SMBIOS Tables
// ! There are some SMBIOS tables after this structure.
//
typedef struct PB_PACKED _mbtag_smbios_tab_t {
	uint8_t major;
	uint8_t minor;
	uint8_t reserved[6];
} mbtag_smbios_tab_t;

//
// Type = 14
// Old ACPI RSDP
//
typedef struct PB_PACKED _mbtag_old_rsdp_t {
	acpi_rsdp rsdp;
} mbtag_old_rsdp_t;

//
// Type = 15
// New ACPI RSDP
//

//
// Type = 16
// Networking Information
//

//
// Type = 17
// EFI Memory Map
// ! There are some EFI memory maps after this structure.
//
typedef struct PB_PACKED _mbtag_efi_mmap_t {
	uint32_t desc_size;
	uint32_t desc_version;
} mbtag_efi_mmap_t;

//
// Type = 18
// EFI Boot Service Not Terminated Indicator
//
typedef struct PB_PACKED _mbtag_efiserv_unterm_t {
} mbtag_efiserv_unterm_t;

//
// Type = 19
// EFI 32-bit Image Handle Pointer
//
typedef struct PB_PACKED _mbtag_efi_himg32_t {
	uint32_t ptr;
} mbtag_efi_himg32_t;

//
// Type = 20
// EFI 64-bit Image Handle Pointer
//
typedef struct PB_PACKED _mbtag_efi_himg64_t {
	uint32_t ptr;
} mbtag_efi_himg64_t;

//
// Type = 21
// Image Load Base Physical Address
//
typedef struct PB_PACKED _mbtag_imgbase_t {
	uint32_t addr;
} mbtag_imgbase_t;

//
// Boot information that initializes in the initial code.
//
extern void *boot_p_info;
extern int boot_mb_magic;

int boot_infoscan();

#endif
