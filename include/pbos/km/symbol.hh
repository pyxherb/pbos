#ifndef _PBOS_KI_KM_SYMBOL_HH_
#define _PBOS_KI_KM_SYMBOL_HH_

#include <pbos/common.h>
#include <cstddef>
#include <type_traits>

PBOS_EXTERN_C_BEGIN

#if defined(__GNUC__) || defined(__clang__)
#define PBOS_SYMBOL_NAME(name)
#else
#define PBOS_SYMBOL_NAME _Pragma("error(\"Symbol name specifying macro is not supported!\")")
#endif

PBOS_EXTERN_C_END

#endif
