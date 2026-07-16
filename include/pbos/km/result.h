#ifndef _PBOS_KM_RESULT_H_
#define _PBOS_KM_RESULT_H_

#include <stdint.h>
#include "panic.h"

typedef enum _km_results {
	KM_RESULT_OK = 0,  // Success

	/// @brief The generic failure code indicating a failure.
	KM_RESULT_FAILED,
	/// @brief Operation not permitted by current permission.
	KM_RESULT_NO_PERM,
	/// @brief Invalid arguments were received.
	KM_RESULT_INVALID_ARGS,
	/// @brief No memory.
	KM_RESULT_NO_MEM,
	/// @brief The generic I/O error code indicating an I/O error.
	KM_RESULT_IO_ERROR,
	/// @brief Requested entity was not found.
	KM_RESULT_NOT_FOUND,
	/// @brief Requested function is unavailable right now.
	KM_RESULT_UNAVAILABLE,
	/// @brief Prematured end of file detected.
	KM_RESULT_EOF,
	/// @brief Input data are malformed.
	KM_RESULT_MALFORMED,
	/// @brief Entity requested to create has already existed.
	KM_RESULT_EXISTED,
	/// @brief No enough entity slot to perform the operation.
	KM_RESULT_NO_SLOT,
	/// @brief Requested operation is not supported.
	KM_RESULT_UNSUPPORTED_OPERATION,
	/// @brief Unsupported executable format.
	KM_RESULT_UNSUPPORTED_EXECFMT,
	/// @brief Memory access violation.
	KM_RESULT_ACCESS_VIOLATION,
	/// @brief The directory to be operated is not empty.
	KM_RESULT_DIR_NOT_EMPTY,
	/// @brief Found unresolved symbols.
	KM_RESULT_UNRESOLVED_SYMBOL,
	/// @brief Requested entity or operation is not processable.
	KM_RESULT_UNPROCESSABLE,
	/// @brief Corresponding page table has not been allocated yet.
	KM_RESULT_PGTAB_NOT_ALLOCATED,
	/// @brief A required dependency was not found.
	KM_RESULT_DEPENDECY_NOT_FOUND,
	/// @brief Cyclic dependency was found.
	KM_RESULT_CYCLIC_DEPENDENCY,
	/// @brief Corresponding device class was not found.
	KM_RESULT_DEVICE_CLASS_NOT_FOUND,
	/// @brief Corresponding device has been removed.
	KM_RESULT_DEVICE_REMOVED,

	/// @brief The operation should be continue to process.
	KM_RESULT_CONTINUE = 0x80000000,
} km_results;

typedef uint32_t km_result_t;

#define KM_SUCCESS(result) (!(result))
#define KM_FAILED(result) (result)

PBOS_FORCEINLINE void km_unwrap_result(km_result_t result) {
	if (KM_FAILED(result))
		km_panic("Unwrap failed!");
}

#define KM_RETURN_IF_FAILED(expr)   \
	do {                            \
		km_result_t _ = (expr);     \
		if (KM_FAILED(_)) return _; \
	} while (false)

#endif
