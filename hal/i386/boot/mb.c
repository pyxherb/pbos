#include "mb.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "misc.h"
#include <hal/i386/display/vga.h>

void *boot_p_info;
int boot_mb_magic;

int boot_infoscan() {
	char errmsg[255];

	void *cur_ptr = boot_p_info;

	// Boot information header data (metadata).
	const mb_info_hdr_t *header = cur_ptr;
	cur_ptr += sizeof(mb_info_hdr_t);

	// Scan for all tags.
	while (true) {
		mbtaghdr_t *tag_header = cur_ptr;
		void *tag_body = cur_ptr + sizeof(mbtaghdr_t);

		switch (tag_header->type) {
			case MB_TAG_BIOS_BOOTDEV:
				break;
			case MB_TAG_BOOT_CMDLINE: {
				mbtag_cmdline_t *tag = tag_body;
				char boot_cmdline[256];

				memset(boot_cmdline, 0, sizeof(*boot_cmdline));
				strncpy(boot_cmdline, &(tag->string), 255);

				break;
			}
			case MB_TAG_loader_name: {
				mbtag_loader_name_t *tag = tag_body;

				memset(ARCH_KARGS_PTR->loader_name, 0, sizeof(ARCH_KARGS_PTR->loader_name));
				strncpy(ARCH_KARGS_PTR->loader_name, &(tag->string), 255);

				break;
			}
			case MB_TAG_BASIC_MEMINFO:
				// Ignored.
				break;
			case MB_TAG_MEMMAP: {
				mbtag_mmap_t *tag = tag_body;

				memset(ARCH_KARGS_PTR->mmaps, ARCH_MEM_END, sizeof(ARCH_KARGS_PTR->mmaps));

				for (size_t i = 0; i < (tag_header->size - sizeof(tag_header) - sizeof(tag)) / tag->sz_entry; ++i) {
					mb_mmap_entry_t *mmap = (mb_mmap_entry_t *)&(tag[1]) + i;

					if (i >= ARCH_MMAP_MAX) {
						boot_seterr("Too many memory maps");
						return -1;
					}

					ARCH_KARGS_PTR->mmaps[i].base = mmap->base;
					ARCH_KARGS_PTR->mmaps[i].size = mmap->length;

					switch (mmap->type) {
						case MB_ARCH_MEM_AVAILABLE:
							ARCH_KARGS_PTR->mmaps[i].type = ARCH_MEM_AVAILABLE;
							break;
						case MB_MEMTYPE_DEFECTIVE:
							ARCH_KARGS_PTR->mmaps[i].type = ARCH_MEM_BAD;
							break;
						case MB_ARCH_MEM_ACPI_INFO:
							ARCH_KARGS_PTR->mmaps[i].type = ARCH_MEM_ACPI;
							break;
						case MB_ARCH_MEM_HIBERNATION:
							ARCH_KARGS_PTR->mmaps[i].type = ARCH_MEM_HIBERNATION;
							break;
						default:
							ARCH_KARGS_PTR->mmaps[i].type = ARCH_MEM_RESERVED;
					}
				}
				break;
			}
			case MB_TAG_FRAMEBUFFER_INFO:
				// Ignored.
				break;
			case MB_TAG_ELF_SYMBOLS:
				// Ignored.
				break;
			case MB_TAG_APM_TABLE: {
				mbtag_apmtab_t *tag = tag_body;

				ARCH_KARGS_PTR->apm_entry.exist = true;
				ARCH_KARGS_PTR->apm_entry.cs = tag->cseg;
				ARCH_KARGS_PTR->apm_entry.len_cs = tag->cseg_len;
				ARCH_KARGS_PTR->apm_entry.ds = tag->dseg;
				ARCH_KARGS_PTR->apm_entry.len_ds = tag->dseg_len;
				ARCH_KARGS_PTR->apm_entry.off_entry = (void *)tag->offset;
				ARCH_KARGS_PTR->apm_entry.flags = tag->flags;

				break;
			}
			case MB_TAG_OLD_ACPI_RSDP: {
				mbtag_old_rsdp_t *tag = tag_body;
				break;
			}
			case MB_TAG_MODULES: {
				mbtag_mod_t *tag = tag_body;

				if (tag->mod_start >= KERNEL_PBASE && tag->mod_start < KERNEL_PTOP) {
					sprintf(errmsg, "A module occupies the load space of the kernel\n\tModule range: %p-%p",
						(void *)tag->mod_start, (void *)tag->mod_end);
					boot_seterr(errmsg);
					return -1;
				}

				if (!strcmp(&(tag->string), INITCAR_MODULE_NAME)) {
					ARCH_KARGS_PTR->initcar_ptr = (void *)tag->mod_start;
					ARCH_KARGS_PTR->initcar_size = tag->mod_end - tag->mod_start;
				} else {
					sprintf(errmsg, "Unknown boot module type '%s'", &(tag->string));
					boot_seterr(errmsg);
					return -1;
				}

				break;
			}
			case MB_TAG_IMAGE_LOADBASE_PADDR:
				// Ignored.
				break;
			case MB_TAG_END:
				goto tagscan_end;
			default: {
				sprintf(errmsg, "Unresolvable boot information tag type 0x%x", tag_header->type);
				boot_seterr(errmsg);
				return -1;
			}
		}

		cur_ptr += tag_header->size;

		if (tag_header->size % 8)
			cur_ptr += 8 - (tag_header->size % 8);
	}

tagscan_end:
	ARCH_KARGS_PTR->magic[0] = KARG_MAGIC0;
	ARCH_KARGS_PTR->magic[1] = KARG_MAGIC1;
	ARCH_KARGS_PTR->magic[2] = KARG_MAGIC2;
	ARCH_KARGS_PTR->magic[3] = KARG_MAGIC3;

	return 0;
}
