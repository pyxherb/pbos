#ifndef _PBOS_KM_KMOD_H_
#define _PBOS_KM_KMOD_H_

#include "proc.h"

PBOS_EXTERN_C_BEGIN

typedef struct _ps_kmod_section_t ps_kmod_section_t;
typedef struct _ps_kmod_t ps_kmod_t;

PBOS_API km_result_t ps_load_kmod(fs_fcb_t *file_fp, ps_kmod_t **kmod_out);

PBOS_API char *ps_get_kmod_name(ps_kmod_t *kmod, size_t *len_out);
PBOS_API void ps_unname_kmod(ps_kmod_t *kmod);
PBOS_NODISCARD PBOS_API km_result_t ps_set_kmod_name(ps_kmod_t *kmod, const char *name, size_t name_len);
///
/// @brief Remove and destroy a kernel module.
///
/// @param kmod Kernel module to be removed.
///
PBOS_API void ps_destroy_kmod(ps_kmod_t *kmod);
PBOS_NODISCARD PBOS_API km_result_t ps_create_kmod(ps_kmod_t **kmod_out);
PBOS_NODISCARD PBOS_API km_result_t ps_add_section_to_kmod(ps_kmod_t *kmod, void *vaddr, size_t size, ps_kmod_section_t **section_out);
PBOS_NODISCARD PBOS_API km_result_t ps_get_section_of_kmod(ps_kmod_t *kmod, void *vaddr, ps_kmod_section_t **section_out);
PBOS_API void ps_remove_section_from_kmod(ps_kmod_t *kmod, ps_kmod_section_t *section);

typedef km_result_t (*ps_kmod_init_fn_t)();
typedef void (*ps_kmod_deinit_fn_t)();
PBOS_API void ps_set_kmod_init_fn(ps_kmod_t *kmod, ps_kmod_init_fn_t init_fn);
PBOS_API void ps_set_kmod_deinit_fn(ps_kmod_t *kmod, ps_kmod_deinit_fn_t deinit_fn);

PBOS_NODISCARD PBOS_API km_result_t ps_register_kernel_symbol(ps_kmod_t *kmod, const char *name, size_t name_len, void *addr, size_t len);
PBOS_NODISCARD PBOS_API km_result_t ps_unregister_kernel_symbol(const char *name, size_t name_len);
PBOS_API void *ps_get_kernel_symbol(const char *name, size_t name_len);

PBOS_EXTERN_C_END

#endif
