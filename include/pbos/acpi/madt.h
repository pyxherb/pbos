#ifndef _PBOS_KN_ACPI_MADT_HH_
#define _PBOS_KN_ACPI_MADT_HH_

#include <pbos/common.h>

#include <pbos/packed.h>

#define ACPI_MADT_TYPE_LAPIC 0x00
#define ACPI_MADT_TYPE_IOAPIC 0x01
#define ACPI_MADT_TYPE_IOAPIC_INT_SRC_OVERRIDE 0x02
#define ACPI_MADT_TYPE_NMI_SRC 0x03
#define ACPI_MADT_TYPE_NMI 0x04
#define ACPI_MADT_TYPE_LAPIC_ADDR_OVERRIDE 0x05
#define ACPI_MADT_TYPE_LX2APIC 0x09

typedef struct _acpi_madt_exdata_t {
	uint32_t lapic_addr;
	uint32_t flags;
} PBOS_PACKED acpi_madt_exdata_t;

typedef struct _acpi_madt_entry_header_t {
	uint8_t entry_type;
	uint8_t record_length;
} PBOS_PACKED acpi_madt_entry_header_t;

typedef struct _acpi_madt_entry_lapic_t {
	acpi_madt_entry_header_t header;
	uint8_t processor_id;
	uint8_t apic_id;
	uint32_t flags;
} PBOS_PACKED acpi_madt_entry_lapic_t;

typedef struct _acpi_madt_entry_ioapic_t {
	acpi_madt_entry_header_t header;
	uint8_t ioapic_id;
	uint8_t reserved;
	uint32_t ioapic_addr;
	uint32_t global_sys_int_base;
} PBOS_PACKED acpi_madt_entry_ioapic_t;

typedef struct _acpi_madt_entry_ioapic_int_src_override_t {
	acpi_madt_entry_header_t header;
	uint8_t bus_src;
	uint8_t irq_src;
} PBOS_PACKED acpi_madt_entry_ioapic_int_src_override_t;

typedef struct _acpi_madt_entry_ioapic_nmi_src_t {
	acpi_madt_entry_header_t header;
	uint8_t nmi_src;
	uint8_t reserved;
	uint16_t flags;
	uint32_t global_sys_src;
} PBOS_PACKED acpi_madt_entry_ioapic_nmi_src_t;

typedef struct _acpi_madt_entry_ioapic_nmi_t {
	acpi_madt_entry_header_t header;
	uint8_t processor_id;
	uint16_t flags;
	uint8_t lint;
} PBOS_PACKED acpi_madt_entry_ioapic_nmi_t;

typedef struct _acpi_madt_entry_lapic_addr_override_t {
	acpi_madt_entry_header_t header;
	uint16_t reserved;
	uint64_t lapic_paddr;
} PBOS_PACKED acpi_madt_entry_lapic_addr_override_t;

typedef struct _acpi_madt_entry_lx2apic_t {
	acpi_madt_entry_header_t header;
	uint16_t reserved;
	uint32_t x2apic_id;
	uint32_t flags;
	uint32_t acpi_id;
} PBOS_PACKED acpi_madt_entry_lx2apic_t;

#include <pbos/packed_end.h>

#endif
