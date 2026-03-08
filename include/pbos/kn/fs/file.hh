#ifndef _PBOS_KN_FS_FILE_HH_
#define _PBOS_KN_FS_FILE_HH_

#include <pbos/fs/file.h>
#include <pbos/kf/list.h>
#include <pbos/km/objmgr.h>
#include <pbos/kfxx/hashmap.hh>
#include "fs.hh"

PBOS_EXTERN_C_BEGIN

class kn_fs_filename_allocator_t : public kfxx::allocator_t {
public:
	PBOS_API kn_fs_filename_allocator_t();
	PBOS_API virtual ~kn_fs_filename_allocator_t();

	PBOS_API virtual size_t inc_ref() noexcept override;
	PBOS_API virtual size_t dec_ref() noexcept override;

	PBOS_API virtual void *alloc(size_t size, size_t alignment) noexcept override;
	PBOS_API virtual void *realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept override;
	PBOS_API virtual void release(void *ptr, size_t size, size_t alignment) noexcept override;

	PBOS_API virtual void *type_identity() const noexcept override;
};

extern kn_fs_filename_allocator_t kn_fs_filename_allocator;

typedef struct _fs_fnode_t : public om_object_t {
	_fs_fnode_t *parent;

	fs_filesys_t *fs;
	fs_filetype_t filetype;

	size_t filename_len;
	char *filename;
} fs_fnode_t;

typedef struct _fs_file_t : public fs_fnode_t {
} fs_file_t;

typedef struct _fs_dir_t : public fs_fnode_t {
} fs_dir_t;

void kn_file_destructor(om_object_t *obj);

PBOS_EXTERN_C_END

#endif
