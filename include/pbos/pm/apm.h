///
/// @file apm.h
/// @author PbOS Project
/// @brief Definitions specified by APM Specification.
/// @version 0.0.1
///
/// @copyright Copyright (c) 2023 PbOS Contributors
/// @see http://download.microsoft.com/download/1/6/1/161ba512-40e2-4cc9-843a-923143f3456c/APMV12.rtf
///
#ifndef _PBOS_PM_APM_H_
#define _PBOS_PM_APM_H_

// Power Management Event (PME) codes.
#define APM_PME_SYSTEM_STANDBY_REQUEST 0x0001
#define APM_PME_SYSTEM_SUSPEND_REQUEST 0x0002
#define APM_PME_NORMAL_RESUME_SYSTEM 0x0003
#define APM_PME_CRITICAL_RESUME_SYSTEM 0x0004
#define APM_PME_BATTERY_LOW 0x0005
#define APM_PME_POWER_STATUS_CHANGE 0x0006
#define APM_PME_UPDATE_TIME 0x0007
#define APM_PME_CRITICAL_SYSTEM_SUSPEND 0x0008
#define APM_PME_USER_SYSTEM_STANDBY_REQUEST 0x0009
#define APM_PME_USER_SYSTEM_SUSPEND_REQUEST 0x000a
#define APM_PME_SYSTEM_STANDBY_RESUME 0x000b
#define APM_PME_CAPABILITIES_CHANGE 0x000c

#define APM_PME_OEM_DEFINED_START 0x200
#define APM_PME_OEM_DEFINED_END 0x2ff

#endif
