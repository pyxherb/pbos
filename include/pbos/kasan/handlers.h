#ifndef _PBOS_KASAN_HANDLERS_H_
#define _PBOS_KASAN_HANDLERS_H_

#include "utils.h"
#include <stddef.h>

PBOS_EXTERN_C_BEGIN

PBOS_NO_ASAN PBOS_API void __asan_register_globals(void *globals, size_t size);
PBOS_NO_ASAN PBOS_API void __asan_unregister_globals(void *globals, size_t size);

PBOS_NO_ASAN PBOS_API void __asan_before_dynamic_init(const char *module_name);
PBOS_NO_ASAN PBOS_API void __asan_after_dynamic_init(void);

PBOS_NO_ASAN PBOS_API void __asan_handle_no_return(void);

PBOS_NO_ASAN PBOS_API void __asan_alloca_poison(void *, size_t size);
PBOS_NO_ASAN PBOS_API void __asan_allocas_unpoison(void *stack_top, void *stack_bottom);

PBOS_NO_ASAN PBOS_API void __asan_load1(void *);
PBOS_NO_ASAN PBOS_API void __asan_store1(void *);

PBOS_NO_ASAN PBOS_API void __asan_load2(void *);
PBOS_NO_ASAN PBOS_API void __asan_store2(void *);

PBOS_NO_ASAN PBOS_API void __asan_load4(void *);
PBOS_NO_ASAN PBOS_API void __asan_store4(void *);

PBOS_NO_ASAN PBOS_API void __asan_load8(void *);
PBOS_NO_ASAN PBOS_API void __asan_store8(void *);

PBOS_NO_ASAN PBOS_API void __asan_load16(void *);
PBOS_NO_ASAN PBOS_API void __asan_store16(void *);

PBOS_NO_ASAN PBOS_API void __asan_loadN(void *, size_t size);
PBOS_NO_ASAN PBOS_API void __asan_storeN(void *, size_t size);

PBOS_NO_ASAN PBOS_API void __asan_load1_noabort(void *);
PBOS_NO_ASAN PBOS_API void __asan_store1_noabort(void *);

PBOS_NO_ASAN PBOS_API void __asan_load2_noabort(void *);
PBOS_NO_ASAN PBOS_API void __asan_store2_noabort(void *);

PBOS_NO_ASAN PBOS_API void __asan_load4_noabort(void *);
PBOS_NO_ASAN PBOS_API void __asan_store4_noabort(void *);

PBOS_NO_ASAN PBOS_API void __asan_load8_noabort(void *);
PBOS_NO_ASAN PBOS_API void __asan_store8_noabort(void *);

PBOS_NO_ASAN PBOS_API void __asan_load16_noabort(void *);
PBOS_NO_ASAN PBOS_API void __asan_store16_noabort(void *);

PBOS_NO_ASAN PBOS_API void __asan_loadN_noabort(void *, size_t size);
PBOS_NO_ASAN PBOS_API void __asan_storeN_noabort(void *, size_t size);

PBOS_NO_ASAN PBOS_API void __asan_report_load1_noabort(void *);
PBOS_NO_ASAN PBOS_API void __asan_report_store1_noabort(void *);

PBOS_NO_ASAN PBOS_API void __asan_report_load2_noabort(void *);
PBOS_NO_ASAN PBOS_API void __asan_report_store2_noabort(void *);

PBOS_NO_ASAN PBOS_API void __asan_report_load4_noabort(void *);
PBOS_NO_ASAN PBOS_API void __asan_report_store4_noabort(void *);

PBOS_NO_ASAN PBOS_API void __asan_report_load8_noabort(void *);
PBOS_NO_ASAN PBOS_API void __asan_report_store8_noabort(void *);

PBOS_NO_ASAN PBOS_API void __asan_report_load16_noabort(void *);
PBOS_NO_ASAN PBOS_API void __asan_report_store16_noabort(void *);

PBOS_NO_ASAN PBOS_API void __asan_report_load_n_noabort(void *, size_t size);
PBOS_NO_ASAN PBOS_API void __asan_report_store_n_noabort(void *, size_t size);

PBOS_NO_ASAN PBOS_API void __asan_set_shadow_00(void *addr, size_t size);

PBOS_NO_ASAN PBOS_API void __asan_set_shadow_f1(void *addr, size_t size);

PBOS_NO_ASAN PBOS_API void __asan_set_shadow_f2(void *addr, size_t size);

PBOS_NO_ASAN PBOS_API void __asan_set_shadow_f3(void *addr, size_t size);

PBOS_NO_ASAN PBOS_API void __asan_set_shadow_f5(void *addr, size_t size);

PBOS_NO_ASAN PBOS_API void __asan_set_shadow_f8(void *addr, size_t size);

PBOS_NO_ASAN PBOS_API void *__asan_memset(void *addr, int c, size_t len);
PBOS_NO_ASAN PBOS_API void *__asan_memmove(void *dest, const void *src, size_t len);
PBOS_NO_ASAN PBOS_API void *__asan_memcpy(void *dest, const void *src, size_t len);

/*PBOS_API void __hwasan_load1_noabort(void *);
PBOS_API void __hwasan_store1_noabort(void *);

PBOS_API void __hwasan_load2_noabort(void *);
PBOS_API void __hwasan_store2_noabort(void *);

PBOS_API void __hwasan_load4_noabort(void *);
PBOS_API void __hwasan_store4_noabort(void *);

PBOS_API void __hwasan_load8_noabort(void *);
PBOS_API void __hwasan_store8_noabort(void *);

PBOS_API void __hwasan_load16_noabort(void *);
PBOS_API void __hwasan_store16_noabort(void *);

PBOS_API void __hwasan_loadN_noabort(void *, size_t size);
PBOS_API void __hwasan_storeN_noabort(void *, size_t size);

PBOS_API void __hwasan_tag_memory(void *, uint8_t tag, size_t size);

PBOS_API void *__hwasan_memset(void *addr, int c, size_t len);
PBOS_API void *__hwasan_memmove(void *dest, const void *src, size_t len);
PBOS_API void *__hwasan_memcpy(void *dest, const void *src, size_t len);*/

PBOS_EXTERN_C_END

#endif
