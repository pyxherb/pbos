#ifndef _PBOS_KI_FS_FILE_HH_
#define _PBOS_KI_FS_FILE_HH_

#include <pbos/fs/file.hh>
#include <pbos/kf/list.h>
#include <pbos/kfxx/hashmap.hh>
#include <pbos/kfxx/string_view.hh>
#include "fs.hh"

PBOS_EXTERN_C_BEGIN

class ki_fs_filename_allocator_t : public kfxx::allocator_t {
public:
	PBOS_PRIVATE ki_fs_filename_allocator_t();
	PBOS_PRIVATE virtual ~ki_fs_filename_allocator_t();

	PBOS_PRIVATE virtual size_t inc_ref() noexcept override;
	PBOS_PRIVATE virtual size_t dec_ref() noexcept override;

	PBOS_PRIVATE virtual void *alloc(size_t size, size_t alignment) noexcept override;
	PBOS_PRIVATE virtual void *realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept override;
	PBOS_PRIVATE virtual void *realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size) noexcept override;
	PBOS_PRIVATE virtual void release(void *ptr, size_t size, size_t alignment) noexcept override;

	PBOS_PRIVATE virtual void *type_identity() const noexcept override;
};

class ki_fs_fnode_allocator_t : public kfxx::allocator_t {
public:
	PBOS_PRIVATE ki_fs_fnode_allocator_t();
	PBOS_PRIVATE virtual ~ki_fs_fnode_allocator_t();

	PBOS_PRIVATE virtual size_t inc_ref() noexcept override;
	PBOS_PRIVATE virtual size_t dec_ref() noexcept override;

	PBOS_PRIVATE virtual void *alloc(size_t size, size_t alignment) noexcept override;
	PBOS_PRIVATE virtual void *realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept override;
	PBOS_PRIVATE virtual void *realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size) noexcept override;
	PBOS_PRIVATE virtual void release(void *ptr, size_t size, size_t alignment) noexcept override;

	PBOS_PRIVATE virtual void *type_identity() const noexcept override;
};

class ki_fs_fcb_allocator_t : public kfxx::allocator_t {
public:
	PBOS_PRIVATE ki_fs_fcb_allocator_t();
	PBOS_PRIVATE virtual ~ki_fs_fcb_allocator_t();

	PBOS_PRIVATE virtual size_t inc_ref() noexcept override;
	PBOS_PRIVATE virtual size_t dec_ref() noexcept override;

	PBOS_PRIVATE virtual void *alloc(size_t size, size_t alignment) noexcept override;
	PBOS_PRIVATE virtual void *realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept override;
	PBOS_PRIVATE virtual void *realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size) noexcept override;
	PBOS_PRIVATE virtual void release(void *ptr, size_t size, size_t alignment) noexcept override;

	PBOS_PRIVATE virtual void *type_identity() const noexcept override;
};

extern ki_fs_filename_allocator_t ki_fs_filename_allocator;
extern ki_fs_fnode_allocator_t ki_fs_fnode_allocator;

typedef struct _fs_fcb_t : public kfxx::rbtree_t<fs_fhandle_t>::node_t {
	fs::fnode_ptr fnode;
	void *exdata = nullptr;

	_fs_fcb_t(fs_fnode_t *fnode);
	~_fs_fcb_t();
} fs_fcb_t;

typedef struct _fs_fnode_t {
	size_t ref_count = 0;

	_fs_fnode_t *parent = nullptr;

	fs_filesys_t *fs = nullptr;
	fs_filetype_t file_type;

	size_t filename_len = 0;
	char *filename = nullptr;

	void *exdata = nullptr;

	_fs_fnode_t(fs_filetype_t file_type);
	~_fs_fnode_t();
} fs_fnode_t;

typedef struct _fs_file_t : public fs_fnode_t {
	_fs_file_t();
} fs_file_t;

typedef struct _fs_dir_t : public fs_fnode_t {
	kfxx::hashmap_t<kfxx::string_view, fs::fnode_ptr> subnodes;

	_fs_dir_t(kfxx::allocator_t *allocator);
} fs_dir_t;

PBOS_NODISCARD km_result_t ki_alloc_fcb(fs_fnode_t *file_in, fs_fcb_t **fcb_out);

void ki_destroy_fnode(fs_fnode_t *fnode);
void ki_destroy_fcb(fs_fcb_t *fcb);

void ki_do_unname_fnode(fs_fnode_t *file);
PBOS_NODISCARD km_result_t ki_do_rename_fnode(fs_fnode_t *file, const char *name, size_t name_len);

PBOS_EXTERN_C_END

#endif
