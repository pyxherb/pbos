#ifndef _PBOS_SE_FILE_H_
#define _PBOS_SE_FILE_H_

#include "user.h"

enum {
	SE_FSC_PERM_READ = 0x00000001,		 // Read
	SE_FSC_PERM_WRITE = 0x00000002,		 // Write
	SE_FSC_PERM_EXEC = 0x00000004,		 // Execute
	SE_FSC_PERM_LIST = 0x00000008,		 // List children
	SE_FSC_PERM_CREATE = 0x00000010,	 // Create
	SE_FSC_PERM_DELETE = 0x00000020,	 // Delete
	SE_FSC_PERM_READ_ACL = 0x00000040,	 // Read ACL
	SE_FSC_PERM_WRITE_ACL = 0x00000040,	 // Write ACL
};

typedef uint32_t se_fsc_perm_t;

/// @brief File Security Context (FSC)
typedef struct _se_fsc_t {
	se_uid_t uid;
	se_fsc_perm_t perms;
} se_fsc_t;

#endif
