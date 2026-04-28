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
	/// @brief The local APIC address.
	uint32_t lapic_addr;
	/// @brief Flags, bit 0 means dual 8259 legacy PICs are installed.
	uint32_t flags;
} PBOS_PACKED acpi_madt_exdata_t;

typedef struct _acpi_madt_entry_header_t {
	/// @brief Type of the entry.
	uint8_t entry_type;
	/// @brief Size in byte of the whole entry.
	uint8_t record_length;
} PBOS_PACKED acpi_madt_entry_header_t;

/// @brief This flag indicates if the processor is enabled.
#define ACPI_MADT_CPU_FLAG_PROCESSOR_ENABLED 0x01
/// @brief This flag indicates if the processor is online capable (disabled but still can be enabled).
#define ACPI_MADT_CPU_FLAG_ONLINE_CAPABLE 0x02

typedef struct _acpi_madt_entry_lapic_t {
	acpi_madt_entry_header_t header;
	/// @brief ACPI processor id.
	uint8_t processor_id;
	/// @brief APIC ID of the processor.
	uint8_t apic_id;
	/// @brief Flags of the CPU, see `ACPI_MADT_CPU_FLAG_XXX` series macros.
	uint32_t flags;
} PBOS_PACKED acpi_madt_entry_lapic_t;

typedef struct _acpi_madt_entry_ioapic_t {
	acpi_madt_entry_header_t header;
	/// @brief ID of the I/O APIC
	uint8_t ioapic_id;
	uint8_t reserved;
	/// @brief Address of the I/O APIC
	uint32_t ioapic_addr;
	/// @brief The global system interrupt base.
	uint32_t global_sys_int_base;
} PBOS_PACKED acpi_madt_entry_ioapic_t;

typedef struct _acpi_madt_entry_ioapic_int_src_override_t {
	acpi_madt_entry_header_t header;
	/// @brief Bus source.
	uint8_t bus_src;
	/// @brief IRQ source.
	uint8_t irq_src;
	/// @brief Global system interrupt.
	uint32_t global_sys_int;
	/// @brief Flags.
	uint16_t flags;
} PBOS_PACKED acpi_madt_entry_ioapic_int_src_override_t;

typedef struct _acpi_madt_entry_ioapic_nmi_src_t {
	acpi_madt_entry_header_t header;
	/// @brief NMI source.
	uint8_t nmi_src;
	uint8_t reserved;
	/// @brief Flags.
	uint16_t flags;
	/// @brief Global system interrupt.
	uint32_t global_sys_int;
} PBOS_PACKED acpi_madt_entry_ioapic_nmi_src_t;

typedef struct _acpi_madt_entry_ioapic_nmi_t {
	acpi_madt_entry_header_t header;
	/// @brief ACPI processor ID, `0xff` means all of the processors.
	uint8_t processor_id;
	/// @brief Flags
	uint16_t flags;
	/// @brief LINT# (see Intel's documentation, 0 or 1)
	uint8_t lint;
} PBOS_PACKED acpi_madt_entry_ioapic_nmi_t;

typedef struct _acpi_madt_entry_lapic_addr_override_t {
	acpi_madt_entry_header_t header;
	uint16_t reserved;
	/// @brief 64-bit physical address of the local APIC
	uint64_t lapic_paddr;
} PBOS_PACKED acpi_madt_entry_lapic_addr_override_t;

typedef struct _acpi_madt_entry_lx2apic_t {
	acpi_madt_entry_header_t header;
	uint16_t reserved;
	/// @brief Processor's local x2APIC ID.
	uint32_t x2apic_id;
	/// @brief Flags of the CPU, see `ACPI_MADT_CPU_FLAG_XXX` series macros.
	uint32_t flags;
	/// @brief ACPI ID.
	uint32_t acpi_id;
} PBOS_PACKED acpi_madt_entry_lx2apic_t;

#include <pbos/packed_end.h>

#endif
