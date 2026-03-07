#ifndef _FREESTDC_C_BASEDEFS_H_
#define _FREESTDC_C_BASEDEFS_H_

#ifdef __cplusplus
#define FREESTDC_EXTERN_C_BEGIN extern "C" {
#define FREESTDC_EXTERN_C_END }
#else
#define FREESTDC_EXTERN_C_BEGIN
#define FREESTDC_EXTERN_C_END
#endif

#ifndef FREESTDC_FORCEINLINE
	#if defined(__GNUC__) || defined(__clang__)
		#define FREESTDC_FORCEINLINE __attribute__((__always_inline__)) inline
	#else
		#define FREESTDC_FORCEINLINE
	#endif
#endif

#endif
