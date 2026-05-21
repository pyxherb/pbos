#include <pbos/km/result.h>

PBOS_EXTERN_C_BEGIN

typedef void (*kxi_ctor_t)();
typedef void (*kxi_dtor_t)();

extern kxi_ctor_t KXI_CTORS_BEGIN[];
extern kxi_ctor_t KXI_CTORS_END[];

extern kxi_dtor_t KXI_DTORS_BEGIN[];
extern kxi_dtor_t KXI_DTORS_END[];

void kxi_call_ctors() {
	const size_t n_ctors = KXI_CTORS_END - KXI_CTORS_BEGIN;
	for (size_t i = 0; i < n_ctors; ++i) {
		KXI_CTORS_BEGIN[i]();
	}
}

void kxi_call_dtors() {
	const size_t n_dtors = KXI_DTORS_END - KXI_DTORS_BEGIN;
	for (size_t i = 0; i < n_dtors; ++i) {
		KXI_DTORS_BEGIN[i]();
	}
}

PBOS_EXTERN_C_END
