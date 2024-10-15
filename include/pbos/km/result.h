#ifndef _PBOS_KM_RESULT_H_
#define _PBOS_KM_RESULT_H_

#include <stdint.h>

#define KM_RESULT_TYPE_SUCCESS 0x00000000  // Succeeded
#define KM_RESULT_TYPE_NOTICE 0x20000000   // Notice
#define KM_RESULT_TYPE_WARNING 0x40000000  // Warning
#define KM_RESULT_TYPE_ERROR 0x60000000	   // Error
#define KM_RESULT_TYPE_FATAL 0x80000000	   // Fatal error

enum {
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
};

typedef uint32_t km_result_t;

#define KM_RESULT_TYPE(result) ((result)&0xe0000000)

#define KM_SUCCEEDED(result) (!(result))
#define KM_NOTICED(result) (KM_RESULT_TYPE(result) == KM_RESULT_TYPE_INFO)
#define KM_WARNED(result) (KM_RESULT_TYPE(result) == KM_RESULT_TYPE_WARNING)
#define KM_FAILED(result) (KM_RESULT_TYPE(result) >= KM_RESULT_TYPE_ERROR)
#define KM_ISCRITICAL(result) (KM_RESULT_TYPE(result) >= KM_RESULT_TYPE_CRITICAL)
#define KM_ISFATAL(result) (KM_RESULT_TYPE(result) >= KM_RESULT_TYPE_FATAL)

#define KM_MAKENOTICE(result) ((result) | KM_RESULT_TYPE_NOTICE)
#define KM_MAKEWARNING(result) ((result) | KM_RESULT_TYPE_WARNING)
#define KM_MAKEERROR(result) ((result) | KM_RESULT_TYPE_ERROR)
#define KM_MAKEFATAL(result) ((result) | KM_RESULT_TYPE_FATAL)

#endif
