#include <pbos/ps/proc.h>
#include "../mm.hh"

PBOS_EXTERN_C_BEGIN

PBOS_PURE PBOS_API size_t kh_get_page_size() {
	return PAGESIZE;
}

PBOS_API void mm_invl_page(void *ptr) {
	arch_invlpg(ptr);
}

PBOS_API bool mm_is_user_space(const void *ptr) {
	return ptr < (void *)0x0000800000000000ULL;
}

PBOS_EXTERN_C_END
