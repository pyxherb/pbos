#ifndef _PBOS_KM_STRING_H_
#define _PBOS_KM_STRING_H_

#include <string.h>
#include "result.h"

typedef void *km_shared_string_handle_t;

PBOS_API km_result_t km_register_shared_string(const char *str, size_t len, const char **str_out, km_shared_string_handle_t *handle_out);
PBOS_API const char *km_lookup_shared_string(km_shared_string_handle_t handle, size_t *length_out);
PBOS_API void km_ref_shared_string(km_shared_string_handle_t handle);
PBOS_API void km_unref_shared_string(km_shared_string_handle_t handle);

#endif
