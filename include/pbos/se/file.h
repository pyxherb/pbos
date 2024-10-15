#ifndef _PBOS_SE_FILE_H_
#define _PBOS_SE_FILE_H_

#include "user.h"

#define SE_FSC_PERM_READ 0x00000001		  // Read
#define SE_FSC_PERM_WRITE 0x00000002	  // Write
#define SE_FSC_PERM_EXEC 0x00000004		  // Execute
#define SE_FSC_PERM_LIST 0x00000008		  // List children
#define SE_FSC_PERM_CREATE 0x00000010	  // Create
#define SE_FSC_PERM_DELETE 0x00000020	  // Delete
#define SE_FSC_PERM_READ_ACL 0x00000040	  // Read ACL
#define SE_FSC_PERM_WRITE_ACL 0x00000040  // Write ACL

typedef uint32_t se_fsc_perm_t;

/// @brief File Security Context (FSC)
typedef struct _se_fsc_t {
	se_uid_t uid;
	se_fsc_perm_t perms;
} se_fsc_t;

#endif
