///
/// @file device.h
/// @author PbOS Contributors
/// @brief Header file containing interfaces involving PCI device management.
/// @date 2026-05-23
///
/// @copyright Copyright (c) 2026 PbOS Project
///
///
#ifndef _PBOS_PCI_DEVICE_H_
#define _PBOS_PCI_DEVICE_H_

#include "mcfg.h"

#define PCI_DEVICE_HEADER_TYPE_GENERAL 0x00
#define PCI_DEVICE_HEADER_TYPE_PCI_TO_PCI_BRIDGE 0x01
#define PCI_DEVICE_HEADER_TYPE_PCI_TO_CARDBUS_BRIDGE 0x02

#define PCI_COMMAND_IO_SPACE 0x0001
#define PCI_COMMAND_MEMORY_BUS 0x0002
#define PCI_COMMAND_BUS_MASTER 0x0004
#define PCI_COMMAND_SPECIAL_CYCLES 0x0008
#define PCI_COMMAND_MEMORY_WRITE_AND_INVALIDATE_ENABLE 0x0010
#define PCI_COMMAND_VGA_PALETTE_SNOOP 0x0020
#define PCI_COMMAND_PARITY_ERROR_RESPONSE 0x0040
#define PCI_COMMAND_SERR_ENABLE 0x0100
#define PCI_COMMAND_FAST_BACK_TO_BACK_ENABLE 0x0200
#define PCI_COMMAND_INTERRUPT_DISABLE 0x0400

#define PCI_STATUS_INTERRUPT_STATUS (1 << 3)
#define PCI_STATUS_CAPABILITIES_LIST (1 << 4)
#define PCI_STATUS_66MHZ_CAPABLE (1 << 5)
#define PCI_STATUS_FAST_BACK_TO_BACK_CAPABLE (1 << 7)
#define PCI_STATUS_MASTER_DATA_PARITY_ERROR (1 << 8)
#define PCI_STATUS_DEVSEL_TIMING_MASK (0b11 << 9)
#define PCI_STATUS_SIGNALED_TARGET_ABORT (1 << 11)
#define PCI_STATUS_RECEIVED_TARGET_ABORT (1 << 12)
#define PCI_STATUS_RECEIVED_MASTER_ABORT (1 << 13)
#define PCI_STATUS_SIGNALED_SYSTEM_ERROR (1 << 14)
#define PCI_STATUS_DETECTED_PARITY_ERROR (1 << 15)

#define PCI_DEVSEL_TIMING_FAST 0b00
#define PCI_DEVSEL_TIMING_MEDIUM 0b01
#define PCI_DEVSEL_TIMING_SLOW 0b10

#define PCI_HEADER_TYPE_TYPE(ht) ((ht) & 0b01111111)
#define PCI_HEADER_TYPE_MF(ht) ((ht) & 0b10000000)

#define PCI_MAKE_HEADER_TYPE(mf, type) ((mf << 7) | ((uint8_t)type))

#define PCI_BIST_COMPLETION_CODE(bist) ((bist) & 0b00001111)
#define PCI_BIST_START_BIST_BIT(bist) ((bist) & 0b01000000)
#define PCI_BIST_IS_CAPABLE(bist) ((bist) & 0b01000000)

#define PCI_MAKE_BIST(capable, start, completion_code) (((capable) << 7) | ((start) << 6) | ((completion_code)))

#define PCI_MEMORY_SPACE_BAR_TYPE(bar) (((bar) >> 1) & 0b11)
#define PCI_MEMORY_SPACE_BAR_PREFETCHABLE(bar) (((bar) >> 3) & 0b1)
#define PCI_MEMORY_SPACE_BAR_BASE_ADDR(bar) ((bar) & 0xfffffff0)
#define PCI_MAKE_MEMORY_SPACE_BAR(type, prefetchable, base_addr) (((type) << 1) | ((prefetchable) << 3) | ((base_addr) & 0xfffffff0))

#define PCI_IO_SPACE_BAR_BASE_ADDR(bar) ((bar) & 0xfffffffc)
#define PCI_MAKE_IO_SPACE_BAR(base_addr) (1 | ((base_addr) & 0xfffffffc))

/// @brief PCI class class code for unclassified devices.
#define PCI_CLASS_CODE_UNCLASSIFIED 0x00
/// @brief PCI class class code for mass storage controllers.
#define PCI_CLASS_CODE_MASS_STORAGE 0x01
/// @brief PCI class class code for network controllers.
#define PCI_CLASS_CODE_NETWORK 0x02
/// @brief PCI class class code for display controllers.
#define PCI_CLASS_CODE_DISPLAY 0x03
/// @brief PCI class class code for multimedia controllers.
#define PCI_CLASS_CODE_MULTIMEDIA 0x04
/// @brief PCI class class code for memory controllers.
#define PCI_CLASS_CODE_MEMORY_CONTROLLER 0x05
/// @brief PCI class class code for bridges.
#define PCI_CLASS_CODE_BRIDGE 0x06
/// @brief PCI class class code for simple communication controllers.
#define PCI_CLASS_CODE_SIMPLE_COMM 0x07
/// @brief PCI class class code for base system peripherials.
#define PCI_CLASS_CODE_SYSTEM_PERIPH 0x08
/// @brief PCI class class code for input device controllers.
#define PCI_CLASS_CODE_INPUT 0x09
/// @brief PCI class class code for docking stations.
#define PCI_CLASS_CODE_DOCKING_STATION 0x0A
/// @brief PCI class class code for processors.
#define PCI_CLASS_CODE_PROCESSOR 0x0B
/// @brief PCI class class code for serial bus contollers.
#define PCI_CLASS_CODE_SERIAL_BUS 0x0C
/// @brief PCI class class code for wireless controllers.
#define PCI_CLASS_CODE_WIRELESS 0x0D
/// @brief PCI class class code for intelligent controllers.
#define PCI_CLASS_CODE_INTELLIGENT_IO 0x0E
/// @brief PCI class class code for satellite communication controllers.
#define PCI_CLASS_CODE_SATELLITE 0x0F
/// @brief PCI class class code for encryption controllers.
#define PCI_CLASS_CODE_ENCRYPTION 0x10
/// @brief PCI class class code for signal processing controllers.
#define PCI_CLASS_CODE_SIGNAL_PROCESSING 0x11
/// @brief PCI class class code for processing accelerator.
#define PCI_CLASS_CODE_PROCESSING_ACCEL 0x12
/// @brief PCI class class code for non-essential instrumentations.
#define PCI_CLASS_CODE_NON_ESSENTIAL 0x13
/// @brief PCI class class code for coprocessors.
#define PCI_CLASS_CODE_COPROCESSOR 0x40
/// @brief Unassigned class code, vendor specific.
#define PCI_CLASS_CODE_UNASSIGNED 0xFF

/// @brief Subclass code for non-VGA-compatible unclassified devices.
#define PCI_SUBCLASS_CODE_UNCLASSIFIED_NON_VGA_COMPATIBLE 0x00
/// @brief Subclass code for VGA-compatible unclassified devices.
#define PCI_SUBCLASS_CODE_UNCLASSIFIED_VGA_COMPATIBLE 0x01

/// @brief Subclass code for SCSI bus controllers.
#define PCI_SUBCLASS_CODE_MASS_STORAGE_SCSI 0x00
/// @brief Subclass code for IDE controllers.
#define PCI_SUBCLASS_CODE_MASS_STORAGE_IDE 0x01
/// @brief Subclass code for floppy disk controllers.
#define PCI_SUBCLASS_CODE_MASS_STORAGE_FLOPPY 0x02
/// @brief Subclass code for IPI bus controllers.
#define PCI_SUBCLASS_CODE_MASS_STORAGE_IPI 0x03
/// @brief Subclass code for RAID controllers.
#define PCI_SUBCLASS_CODE_MASS_STORAGE_RAID 0x04
/// @brief Subclass code for ATA controllers.
#define PCI_SUBCLASS_CODE_MASS_STORAGE_ATA 0x05
/// @brief Subclass code for SATA controllers.
#define PCI_SUBCLASS_CODE_MASS_STORAGE_SATA 0x06
/// @brief Subclass code for SAS controllers.
#define PCI_SUBCLASS_CODE_MASS_STORAGE_SAS 0x07
/// @brief Subclass code for NVMe controllers.
#define PCI_SUBCLASS_CODE_MASS_STORAGE_NVME 0x08
/// @brief Subclass code for other mass storage controllers.
#define PCI_SUBCLASS_CODE_MASS_STORAGE_OTHER 0x80

/// @brief Subclass code for Ethernet controllers.
#define PCI_SUBCLASS_CODE_NETWORK_ETHERNET 0x00
/// @brief Subclass code for token ring controllers.
#define PCI_SUBCLASS_CODE_NETWORK_TOKEN_RING 0x01
/// @brief Subclass code for FDDI controllers.
#define PCI_SUBCLASS_CODE_NETWORK_FDDI 0x02
/// @brief Subclass code for ATM controllers.
#define PCI_SUBCLASS_CODE_NETWORK_ATM 0x03
/// @brief Subclass code for ISDN controllers.
#define PCI_SUBCLASS_CODE_NETWORK_ISDN 0x04
/// @brief Subclass code for WorldFip controllers.
#define PCI_SUBCLASS_CODE_NETWORK_WORLDFIP 0x05
/// @brief Subclass code for PICMG 2.14 multi-computing controllers.
#define PCI_SUBCLASS_CODE_NETWORK_PICMG 0x06
/// @brief Subclass code for InfiniBand controllers.
#define PCI_SUBCLASS_CODE_NETWORK_INFINIBAND 0x07
/// @brief Subclass code for fabric controllers.
#define PCI_SUBCLASS_CODE_NETWORK_FABRIC 0x08
/// @brief Subclass code for other network controllers.
#define PCI_SUBCLASS_CODE_NETWORK_OTHER 0x80

/// @brief Subclass code for VGA compatible display controllers.
#define PCI_SUBCLASS_CODE_DISPLAY_VGA 0x00
/// @brief Subclass code for XGA compatible display controllers.
#define PCI_SUBCLASS_CODE_DISPLAY_XGA 0x01
/// @brief Subclass code for 3D display controllers (not VGA-compatible!).
#define PCI_SUBCLASS_CODE_DISPLAY_3D 0x02
/// @brief Subclass code for other display controllers.
#define PCI_SUBCLASS_CODE_DISPLAY_OTHER 0x80

/// @brief Subclass code for multimedia video controllers.
#define PCI_SUBCLASS_CODE_MULTIMEDIA_VIDEO 0x00
/// @brief Subclass code for multimedia audio controllers.
#define PCI_SUBCLASS_CODE_MULTIMEDIA_AUDIO 0x01
/// @brief Subclass code for multimedia telephony controllers.
#define PCI_SUBCLASS_CODE_MULTIMEDIA_TELEPHONY 0x02
/// @brief Subclass code for audio devices.
#define PCI_SUBCLASS_CODE_MULTIMEDIA_AUDIO_DEVICE 0x03
/// @brief Subclass code for other multimedia controllers.
#define PCI_SUBCLASS_CODE_MULTIMEDIA_OTHER 0x80

/// @brief Subclass code for RAM controllers.
#define PCI_SUBCLASS_CODE_MEMORY_RAM 0x00
/// @brief Subclass code for flash controllers.
#define PCI_SUBCLASS_CODE_MEMORY_FLASH 0x01
/// @brief Subclass code for other memory controllers.
#define PCI_SUBCLASS_CODE_MEMORY_OTHER 0x80

/// @brief Subclass code for host bridges.
#define PCI_SUBCLASS_CODE_BRIDGE_HOST 0x00
/// @brief Subclass code for ISA bridges.
#define PCI_SUBCLASS_CODE_BRIDGE_ISA 0x01
/// @brief Subclass code for EISA bridges.
#define PCI_SUBCLASS_CODE_BRIDGE_EISA 0x02
/// @brief Subclass code for MCA bridges.
#define PCI_SUBCLASS_CODE_BRIDGE_MCA 0x03
/// @brief Subclass code for first kind of PCI-to-PCI bridges.
#define PCI_SUBCLASS_CODE_BRIDGE_PCI_TO_PCI_1 0x04
/// @brief Subclass code for PCMCIA bridges.
#define PCI_SUBCLASS_CODE_BRIDGE_PCMCIA 0x05
/// @brief Subclass code for NuBus bridges.
#define PCI_SUBCLASS_CODE_BRIDGE_NUBUS 0x06
/// @brief Subclass code for CardBus bridges.
#define PCI_SUBCLASS_CODE_BRIDGE_CARDBUS 0x07
/// @brief Subclass code for RACEway bridges.
#define PCI_SUBCLASS_CODE_BRIDGE_RACEWAY 0x08
/// @brief Subclass code for second kind of PCI-to-PCI bridges.
#define PCI_SUBCLASS_CODE_BRIDGE_PCI_TO_PCI_2 0x09
/// @brief Subclass code for InfiniBand-to-PCI host bridges.
#define PCI_SUBCLASS_CODE_BRIDGE_INFINIBAND_PCI 0x0A
/// @brief Subclass code for other bridges.
#define PCI_SUBCLASS_CODE_BRIDGE_OTHER 0x80

/// @brief Subclass code for serial controllers.
#define PCI_SUBCLASS_CODE_SIMPLE_COMM_SERIAL 0x00
/// @brief Subclass code for parallel controllers.
#define PCI_SUBCLASS_CODE_SIMPLE_COMM_PARALLEL 0x01
/// @brief Subclass code for multiport serial controllers.
#define PCI_SUBCLASS_CODE_SIMPLE_COMM_MULTIPORT 0x02
/// @brief Subclass code for modems.
#define PCI_SUBCLASS_CODE_SIMPLE_COMM_MODEM 0x03
/// @brief Subclass code for GPIB (IEEE 488.1/2) controllers.
#define PCI_SUBCLASS_CODE_SIMPLE_COMM_GPIB 0x04
/// @brief Subclass code for smart card controllers.
#define PCI_SUBCLASS_CODE_SIMPLE_COMM_SMARTCARD 0x05
/// @brief Subclass code for other simple communication controllers.
#define PCI_SUBCLASS_CODE_SIMPLE_COMM_OTHER 0x80

/// @brief Subclass code for programmable interrupt controllers (PIC).
#define PCI_SUBCLASS_CODE_SYSTEM_PIC 0x00
/// @brief Subclass code for DMA controllers.
#define PCI_SUBCLASS_CODE_SYSTEM_DMA 0x01
/// @brief Subclass code for timers.
#define PCI_SUBCLASS_CODE_SYSTEM_TIMER 0x02
/// @brief Subclass code for RTC controllers.
#define PCI_SUBCLASS_CODE_SYSTEM_RTC 0x03
/// @brief Subclass code for PCI hot-plug controllers.
#define PCI_SUBCLASS_CODE_SYSTEM_HOTPLUG 0x04
/// @brief Subclass code for SD host controllers.
#define PCI_SUBCLASS_CODE_SYSTEM_SD 0x05
/// @brief Subclass code for IOMMUs.
#define PCI_SUBCLASS_CODE_SYSTEM_IOMMU 0x06
/// @brief Subclass code for other base system peripherials.
#define PCI_SUBCLASS_CODE_SYSTEM_OTHER 0x80

/// @brief Subclass code for keyboard controllers.
#define PCI_SUBCLASS_CODE_INPUT_KEYBOARD 0x00
/// @brief Subclass code for digitizer pen controllers.
#define PCI_SUBCLASS_CODE_INPUT_DIGITIZER_PEN 0x01
/// @brief Subclass code for mouse controllers.
#define PCI_SUBCLASS_CODE_INPUT_MOUSE 0x02
/// @brief Subclass code for scanner controllers.
#define PCI_SUBCLASS_CODE_INPUT_SCANNER 0x03
/// @brief Subclass code for gameport controllers.
#define PCI_SUBCLASS_CODE_INPUT_GAMEPORT 0x04
/// @brief Subclass code for other input device controllers.
#define PCI_SUBCLASS_CODE_INPUT_OTHER 0x80

/// @brief Subclass code for generic docking stations.
#define PCI_SUBCLASS_CODE_DOCKING_GENERIC 0x00
/// @brief Subclass code for other docking stations.
#define PCI_SUBCLASS_CODE_DOCKING_OTHER 0x80

/// @brief Subclass code for i386 processors.
#define PCI_SUBCLASS_CODE_PROCESSOR_386 0x00
/// @brief Subclass code for i486 processors.
#define PCI_SUBCLASS_CODE_PROCESSOR_486 0x01
/// @brief Subclass code for Pentium processors.
#define PCI_SUBCLASS_CODE_PROCESSOR_PENTIUM 0x02
/// @brief Subclass code for Alpha processors.
#define PCI_SUBCLASS_CODE_PROCESSOR_ALPHA 0x10
/// @brief Subclass code for PowerPC processors.
#define PCI_SUBCLASS_CODE_PROCESSOR_POWERPC 0x20
/// @brief Subclass code for MIPS processors.
#define PCI_SUBCLASS_CODE_PROCESSOR_MIPS 0x30
/// @brief Subclass code for coprocessors.
#define PCI_SUBCLASS_CODE_PROCESSOR_CO 0x40
/// @brief Subclass code for other processors.
#define PCI_SUBCLASS_CODE_PROCESSOR_OTHER 0x80

/// @brief Subclass code for FireWire (IEEE 1394) controllers.
#define PCI_SUBCLASS_CODE_SERIAL_BUS_FIREWIRE 0x00
/// @brief Subclass code for ACCESS bus controllers.
#define PCI_SUBCLASS_CODE_SERIAL_BUS_ACCESS 0x01
/// @brief Subclass code for SSA controllers.
#define PCI_SUBCLASS_CODE_SERIAL_BUS_SSA 0x02
/// @brief Subclass code for USB controllers.
#define PCI_SUBCLASS_CODE_SERIAL_BUS_USB 0x03
/// @brief Subclass code for fibre channels.
#define PCI_SUBCLASS_CODE_SERIAL_BUS_FIBRE_CHANNEL 0x04
/// @brief Subclass code for SMBus controllers.
#define PCI_SUBCLASS_CODE_SERIAL_BUS_SMBUS 0x05
/// @brief Subclass code for InfiniBand controllers.
#define PCI_SUBCLASS_CODE_SERIAL_BUS_INFINIBAND 0x06
/// @brief Subclass code for IPMI interfaces.
#define PCI_SUBCLASS_CODE_SERIAL_IPMI_INTERFACE 0x07
/// @brief Subclass code for SERCOS interfaces (IEC 61491).
#define PCI_SUBCLASS_CODE_SERIAL_SERCOS_INTERFACE 0x08
/// @brief Subclass code for CANbus controllers.
#define PCI_SUBCLASS_CODE_SERIAL_CANBUS 0x09
/// @brief Subclass code for other serial bus controllers.
#define PCI_SUBCLASS_CODE_SERIAL_BUS_OTHER 0x80

/// @brief Subclass code for iRDA compatible wireless controllers.
#define PCI_SUBCLASS_CODE_WIRELESS_IRDA 0x00
/// @brief Subclass code for Infra-red (IR) wireless controllers.
#define PCI_SUBCLASS_CODE_WIRELESS_IR 0x01
/// @brief Subclass code for RF controllers.
#define PCI_SUBCLASS_CODE_WIRELESS_RF_CONTROLLER 0x10
/// @brief Subclass code for bluetooth controllers.
#define PCI_SUBCLASS_CODE_WIRELESS_BLUETOOTH 0x11
/// @brief Subclass code for broadband controllers.
#define PCI_SUBCLASS_CODE_WIRELESS_BROADBAND 0x12
/// @brief Subclass code for 802.1a ethernet controllers.
#define PCI_SUBCLASS_CODE_WIRELESS_80211A 0x20
/// @brief Subclass code for 802.1b ethernet controllers.
#define PCI_SUBCLASS_CODE_WIRELESS_80211B 0x21
/// @brief Subclass code for other wireless controllers.
#define PCI_SUBCLASS_CODE_WIRELESS_OTHER 0x80

/// @brief Subclass code for intelligent I20 controllers.
#define PCI_SUBCLASS_CODE_INTELLIGENT_IO_I2O 0x00

/// @brief Subclass code for satellite TV controllers.
#define PCI_SUBCLASS_CODE_SATELLITE_TV 0x00
/// @brief Subclass code for satellite audio controllers.
#define PCI_SUBCLASS_CODE_SATELLITE_AUDIO 0x01
/// @brief Subclass code for satellite voice controllers.
#define PCI_SUBCLASS_CODE_SATELLITE_VOICE 0x03
/// @brief Subclass code for satellite data controllers.
#define PCI_SUBCLASS_CODE_SATELLITE_DATA 0x04

/// @brief Subclass code for network and computing encryptor/decryptor.
#define PCI_SUBCLASS_CODE_ENCRYPTION_NETWORK 0x00
/// @brief Subclass code for entertainment encryptor/decryptor.
#define PCI_SUBCLASS_CODE_ENCRYPTION_ENTERTAINMENT 0x10
/// @brief Subclass code for other encryptor/decryptor.
#define PCI_SUBCLASS_CODE_ENCRYPTION_OTHER 0x80

/// @brief Subclass code for DPIO modules.
#define PCI_SUBCLASS_CODE_SIGNAL_PROCESSING_DPIO 0x00
/// @brief Subclass code for performance counter.
#define PCI_SUBCLASS_CODE_SIGNAL_PROCESSING_PERF_COUNTER 0x00
/// @brief Subclass code for communication synchronizer.
#define PCI_SUBCLASS_CODE_SIGNAL_PROCESSING_COMM_SYNC 0x10
/// @brief Subclass code for signal processing management.
#define PCI_SUBCLASS_CODE_SIGNAL_PROCESSING_SIGNAL_PROCESSING_MAN 0x20
/// @brief Subclass code for other signal processing controllers.
#define PCI_SUBCLASS_CODE_SIGNAL_PROCESSING_OTHER 0x80

/// @brief Prog IF for ISA compatibility mode-only IDE controllers.
#define PCI_PROG_IF_IDE_ISA_COMPATIBILITY_MODE 0x00
/// @brief Prog IF for native mode-only IDE controllers.
#define PCI_PROG_IF_IDE_NATIVE_MODE 0x05
/// @brief Prog IF for ISA compatibility IDE controllers, which supports switching to PCI native mode.
#define PCI_PROG_IF_IDE_ISA_COMPATIBILITY_MODE_SWITCHABLE 0x0a
/// @brief Prog IF for native IDE controllers, which supports switching to ISA compatibility native mode.
#define PCI_PROG_IF_IDE_NATIVE_MODE_SWITCHABLE 0x0f
/// @brief Prog IF for ISA compatibility IDE controllers, which supports bus mastering.
#define PCI_PROG_IF_IDE_ISA_COMPATIBILITY_MODE_MASTERING 0x80
/// @brief Prog IF for native IDE controllers, which supports bus mastering.
#define PCI_PROG_IF_IDE_NATIVE_MODE_MASTERING 0x85
/// @brief Prog IF for ISA compatibility IDE controllers, which supports switching to PCI native mode and bus mastering.
#define PCI_PROG_IF_IDE_ISA_COMPATIBILITY_MODE_SWITCHABLE_MASTERING 0x8a
/// @brief Prog IF for native IDE controllers, which supports switching to ISA compatibility native mode and bus mastering.
#define PCI_PROG_IF_IDE_NATIVE_MODE_SWITCHABLE_MASTERING 0x8f

/// @brief Prog IF for single DMA ATA controllers.
#define PCI_PROG_IF_ATA_SINGLE_DMA 0x20
/// @brief Prog IF for chained DMA ATA controllers.
#define PCI_PROG_IF_ATA_CHAINED_DMA 0x30

/// @brief Prog IF for SATA controllers with vendor-specific interfaces.
#define PCI_PROG_IF_SATA_VENDOR_SPECIFIC 0x00
/// @brief Prog IF for SATA controllers with AHCI 1.0 interface.
#define PCI_PROG_IF_SATA_AHCI 0x01
/// @brief Prog IF for SATA controllers with serial storage bus.
#define PCI_PROG_IF_SATA_SERIAL_STORAGE_BUS 0x02

/// @brief Prog IF for SAS controllers.
#define PCI_PROG_IF_SAS_SAS 0x00
/// @brief Prog IF for SAS controllers with serial storage bus.
#define PCI_PROG_IF_SAS_SERIAL_STORAGE_BUS 0x01

/// @brief Prog IF for NVMHCI controllers.
#define PCI_PROG_IF_NVME_NVMHCI 0x01
/// @brief Prog IF for NVMe controllers.
#define PCI_PROG_IF_NVME_NVME 0x02

/// @brief Prog IF for VGA controllers.
#define PCI_PROG_IF_VGA_VGA 0x00
/// @brief Prog IF for 8514-compatible controllers.
#define PCI_PROG_IF_VGA_8514 0x01

/// @brief Prog IF for normal-decode first kind of PCI-to-PCI bridges.
#define PCI_PROG_IF_PCI_TO_PCI_1_NORMAL_DECODE 0x00
/// @brief Prog IF for subtractive-decode first kind of PCI-to-PCI bridges.
#define PCI_PROG_IF_PCI_TO_PCI_1_SUB_DECODE 0x01

/// @brief Prog IF for transparent mode RACEway bridges.
#define PCI_PROG_IF_RACEWAY_TRANSPARENT_MODE 0x00
/// @brief Prog IF for endpoint mode RACEway bridges.
#define PCI_PROG_IF_RACEWAY_ENDPOINT_MODE 0x01

/// @brief Prog IF for semi-transparent, primary bus second kind of PCI-to-PCI bridges.
#define PCI_PROG_IF_PCI_TO_PCI_1_PRIMARY 0x40
/// @brief Prog IF for semi-transparent, secondary bus second kind of PCI-to-PCI bridges.
#define PCI_PROG_IF_PCI_TO_PCI_1_SECONDARY 0x80

/// @brief Prog IF for 8250-compatible serial ports.
#define PCI_PROG_IF_SERIAL_8250 0x00
/// @brief Prog IF for 16450-compatible serial ports.
#define PCI_PROG_IF_SERIAL_16450 0x01
/// @brief Prog IF for 16550-compatible serial ports.
#define PCI_PROG_IF_SERIAL_16550 0x02
/// @brief Prog IF for 16650-compatible serial ports.
#define PCI_PROG_IF_SERIAL_16650 0x03
/// @brief Prog IF for 16750-compatible serial ports.
#define PCI_PROG_IF_SERIAL_16750 0x04
/// @brief Prog IF for 16850-compatible serial ports.
#define PCI_PROG_IF_SERIAL_16850 0x05
/// @brief Prog IF for 16950-compatible serial ports.
#define PCI_PROG_IF_SERIAL_16950 0x06

/// @brief Prog IF for standard parallel ports.
#define PCI_PROG_IF_PARALLEL_STANDARD 0x00
/// @brief Prog IF for bi-directional parallel ports.
#define PCI_PROG_IF_PARALLEL_BIDIR 0x01
/// @brief Prog IF for ECP 1.X compliant parallel ports.
#define PCI_PROG_IF_PARALLEL_ECP 0x02
/// @brief Prog IF for IEEE 1284 controllers.
#define PCI_PROG_IF_PARALLEL_IEEE1284 0x03
/// @brief Prog IF for IEEE 1284 target devices.
#define PCI_PROG_IF_PARALLEL_IEEE1284_TARGET 0xfe

/// @brief Prog IF for generic modems.
#define PCI_PROG_IF_MODEM_GENERIC 0x00
/// @brief Prog IF for Hayes 16450-compatible interface modems.
#define PCI_PROG_IF_MODEM_HAYES_16450 0x01
/// @brief Prog IF for Hayes 16550-compatible interface modems.
#define PCI_PROG_IF_MODEM_HAYES_16550 0x02
/// @brief Prog IF for Hayes 16550-compatible interface modems.
#define PCI_PROG_IF_MODEM_HAYES_16650 0x03
/// @brief Prog IF for Hayes 16750-compatible interface modems.
#define PCI_PROG_IF_MODEM_HAYES_16750 0x04

/// @brief Prog IF for 8259-compatible PIC.
#define PCI_PROG_IF_PIC_8259 0x00
/// @brief Prog IF for ISA-compatible PIC.
#define PCI_PROG_IF_PIC_ISA 0x01
/// @brief Prog IF for EISA-compatible PIC.
#define PCI_PROG_IF_PIC_EISA 0x02
/// @brief Prog IF for I/O APIC.
#define PCI_PROG_IF_PIC_IO_APIC 0x10
/// @brief Prog IF for I/O xAPIC.
#define PCI_PROG_IF_PIC_IO_XAPIC 0x20

/// @brief Prog IF for 8237-compatible PIC.
#define PCI_PROG_IF_DMA_8237 0x00
/// @brief Prog IF for ISA-compatible PIC.
#define PCI_PROG_IF_DMA_ISA 0x01
/// @brief Prog IF for EISA-compatible PIC.
#define PCI_PROG_IF_DMA_EISA 0x02

/// @brief Prog IF for 8254-compatible timer.
#define PCI_PROG_IF_TIMER_8254 0x00
/// @brief Prog IF for ISA-compatible timer.
#define PCI_PROG_IF_TIMER_ISA 0x01
/// @brief Prog IF for EISA-compatible timer.
#define PCI_PROG_IF_TIMER_EISA 0x02
/// @brief Prog IF for HPET timer.
#define PCI_PROG_IF_TIMER_HPET 0x03

/// @brief Prog IF for generic RTC.
#define PCI_PROG_IF_RTC_GENERIC 0x00
/// @brief Prog IF for ISA-compatible RTC.
#define PCI_PROG_IF_RTC_ISA 0x01

/// @brief Prog IF for generic gameport.
#define PCI_PROG_IF_GAMEPORT_GENERIC 0x00
/// @brief Prog IF for extended gameport.
#define PCI_PROG_IF_GAMEPORT_EXTENDED 0x10

/// @brief Prog IF for generic IEEE1394 controller.
#define PCI_PROG_IF_FIREWIRE_GENERIC 0x00
/// @brief Prog IF for OHCI IEEE1394 controller.
#define PCI_PROG_IF_FIREWIRE_EXTENDED 0x10

/// @brief Prog IF for UHCI USB controller.
#define PCI_PROG_IF_USB_UHCI 0x00
/// @brief Prog IF for OHCI USB controller.
#define PCI_PROG_IF_USB_OHCI 0x10
/// @brief Prog IF for EHCI USB controller.
#define PCI_PROG_IF_USB_EHCI 0x20
/// @brief Prog IF for XHCI USB controller.
#define PCI_PROG_IF_USB_XHCI 0x30
/// @brief Prog IF for unspecified kind of USB controller.
#define PCI_PROG_IF_USB_UNSPECIFIED 0x80
/// @brief Prog IF for USB devices.
#define PCI_PROG_IF_USB_DEVICE 0xfe

/// @brief Prog IF for SMIC IPMI interface.
#define PCI_PROG_IF_IPMI_SMIC 0x00
/// @brief Prog IF for keyboard controller styled IPMI interface.
#define PCI_PROG_IF_IPMI_KEYBOARD 0x01
/// @brief Prog IF for block transfer IPMI interface.
#define PCI_PROG_IF_IPMI_BLOCK_TRANSFER 0x02

typedef struct _pci_device_common_header_t {
	/// @brief Vendor ID, allocated by PCI-SIG, invalid if valued `0xffff`.
	uint16_t vendor_id;
	/// @brief Device ID, allocated by the vendor.
	uint16_t device_id;
	/// @brief Command register used for controlling the device.
	uint16_t command;
	/// @brief Register to for recording status information for PCI bus related events.
	uint16_t status;
	/// @brief Device's revision, allocated by the vendor.
	uint8_t rev_id;
	/// @brief Programming Interace Byte, a read-only register that specifies register-level interface the device has.
	uint8_t prog_if;
	/// @brief A read-only register which specifies function the device performs.
	uint8_t subclass;
	/// @brief A read-only register which specifies kind of function the device performs.
	uint8_t class_code;
	/// @brief The system cache line size. The device will act as if 0 is set if unsupported value is written.
	uint8_t cache_line_size;
	/// @brief Latency timer in units of PCI bus clocks.
	uint8_t latency_timer;
	/// @brief Header type of the device.
	uint8_t header_type;
	/// @brief Register used for representing BIST status and controlling device BIST.
	uint8_t bist;
} pci_device_common_header_t;

typedef struct _pci_general_device_header_t {
	pci_device_common_header_t common_header;
	uint32_t bar0;
	uint32_t bar1;
	uint32_t bar2;
	uint32_t bar3;
	uint32_t bar4;
	uint32_t bar5;
	uint32_t bar6;
	uint32_t cardbus_cis_ptr;
	uint16_t subsystem_vendor_id;
	uint16_t subsystem_id;
	uint32_t expansion_rom_base_addr;
	uint8_t capabilities_ptr;
	uint8_t reserved[7];
	uint8_t interrupt_line;
	uint8_t interrupt_pin;
	uint8_t min_grant;
	uint8_t max_latency;
} pci_general_device_header_t;

typedef struct _pci_pci_to_pci_bridge_header_t {
	pci_device_common_header_t common_header;
	uint32_t bar0;
	uint32_t bar1;
	uint16_t subordinate_bus_num;
	uint16_t secondary_latency_timer;
	uint32_t secondary_status;
	uint32_t memory_limit;
	uint32_t prefetchable_memory_limit;
	uint32_t prefetchable_base_upper32;
	uint32_t prefetchable_limit_upper32;
	uint32_t io_limit_upper16;
	uint32_t reserved;
	uint32_t expansion_rom_base_addr;
	uint32_t bridge_control;
} pci_pci_to_pci_bridge_header_t;

typedef struct _pci_pci_to_cardbus_bridge_header_t {
	pci_device_common_header_t common_header;
	union {
		uint32_t cardbus_socket;
		uint32_t exca_base_addr;
	};
	uint8_t capabilities_list_off;
	uint8_t reserved;
	uint16_t secondary_status;
	uint8_t pci_bus_num;
	uint8_t cardbus_bus_num;
	uint8_t subordinate_bus_num;
	uint8_t cardbus_latency_timer;
	uint32_t memory_base_addr0;
	uint32_t memory_limit0;
	uint32_t memory_base_addr1;
	uint32_t memory_limit1;
	uint32_t io_base_addr0;
	uint32_t io_limit0;
	uint32_t io_base_addr1;
	uint32_t io_limit1;
	uint8_t interrupt_line;
	uint8_t interrupt_pin;
	uint16_t bridge_control;
	uint16_t subsystem_device_id;
	uint16_t subsystem_vendor_id;
	uint32_t pc_card_legacy_mode_base_addr;
} pci_pci_to_cardbus_bridge_header_t;

#endif
