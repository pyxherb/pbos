#ifndef _PBOS_KM_RESULT_H_
#define _PBOS_KM_RESULT_H_

#include <stdint.h>
#include "panic.h"

typedef enum _km_results {
	KM_RESULT_OK = 0,  // Success

	KM_RESULT_FAILED,				  // Failed due to unknown errors
	KM_RESULT_NO_PERM,				  // No permission
	KM_RESULT_INVALID_ARGS,			  // Bad arguments
	KM_RESULT_NO_MEM,				  // No memory
	KM_RESULT_IO_ERROR,				  // I/O error
	KM_RESULT_NOT_FOUND,			  // Not found
	KM_RESULT_UNAVAILABLE,			  // Unavailable
	KM_RESULT_EOF,					  // End of file
	KM_RESULT_INVALID_FMT,			  // Invalid format
	KM_RESULT_INVALID_ADDR,			  // Invalid address
	KM_RESULT_EXISTED,				  // Existed
	KM_RESULT_DEPRECATED,			  // Deprecated
	KM_RESULT_INTERNAL_ERROR,		  // Internal error
	KM_RESULT_NO_SLOT,				  // No free slot
	KM_RESULT_UNSUPPORTED_OPERATION,  // Unsupported operation
	KM_RESULT_UNSUPPORTED_EXECFMT,	  // Unsupport executable format
	KM_RESULT_ACCESS_VIOLATION,		  // Access violation
	KM_RESULT_CHILD_FORKED,			  // Child forked
} km_results;

typedef uint32_t km_result_t;

#define KM_SUCCEEDED(result) (!(result))
#define KM_FAILED(result) (result)

#define KM_MAKEERROR(result) ((result))

PBOS_FORCEINLINE void km_unwrap_result(km_result_t result) {
	if(KM_FAILED(result))
		km_panic("Unwrap failed!");
}

#endif
