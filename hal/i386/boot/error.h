#ifndef HAL_I386_BOOT_ERROR_H
#define HAL_I386_BOOT_ERROR_H

const char* boot_geterr();
void boot_seterr(const char* errstr);

void boot_panic(const char* msg, ...);

#endif
