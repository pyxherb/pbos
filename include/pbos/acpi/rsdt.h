#ifndef _PBOS_KI_ACPI_RSDP_HH_
#define _PBOS_KI_ACPI_RSDP_HH_

#include <pbos/common.h>

#include <pbos/packed.h>

#define ACPI_RSDP_SIGNATURE_INITIALIZER { 'R', 'S', 'D', ' ', 'P', 'T', 'R', '\0'}

typedef struct _acpi_rsdp_t {
	/// @brief Magic signature of the RSDP.
	char signature[8];
	/// @brief Checksum for the first 20 bytes of RSDP.
	uint8_t checksum;
	/// @brief OEM ID.
	char oem_id[6];
	/// @brief The RSDP revision, should be 0.
	uint8_t revision;
	/// @brief 32-bit physical address to the RSDT.
	uint32_t rsdt_addr;
} PBOS_PACKED acpi_rsdp_t;

typedef struct _acpi_xsdp_t {
	/// @brief The RSDP header.
	acpi_rsdp_t rsdp_header;

	/// @brief Size of the XSDP.
	uint32_t length;
	/// @brief 64-bit physical address to the XSDT.
	uint64_t xsdt_addr;
	/// @brief Checksum for the whole table.
	uint8_t extended_checksum;
	uint8_t reserved[3];
} PBOS_PACKED acpi_xsdp_t;

typedef struct _acpi_sdt_header_t {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} PBOS_PACKED acpi_sdt_header_t;

#include <pbos/packed_end.h>

#endif
