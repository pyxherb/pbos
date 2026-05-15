#include <pbos/kf/hash.h>
#include <pbos/kf/string.h>
#include <pbos/mm/mm.h>
#include <pbos/ki/fs/file.hh>
#include <pbos/ki/fs/fs.hh>
#include <pbos/ki/km/symbol.hh>
#include <pbos/kf/atomic.h>

PBOS_EXTERN_C_BEGIN

PBOS_API fs_fnode_t *fs_file_of_fcb(fs_fcb_t *fcb) {
	return fcb->fnode.get();
}

ki_fs_filename_allocator_t::ki_fs_filename_allocator_t() {
}

ki_fs_filename_allocator_t::~ki_fs_filename_allocator_t() {
}

size_t ki_fs_filename_allocator_t::inc_ref() noexcept {
	return 0;
}

size_t ki_fs_filename_allocator_t::dec_ref() noexcept {
	return 0;
}

void *ki_fs_filename_allocator_t::alloc(size_t size, size_t alignment) noexcept {
	return mm_kalloc(size, alignment);
}

void *ki_fs_filename_allocator_t::realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	// TODO: Implement mm_krealloc and rewrite this with it.
	return mm_krealloc(ptr, new_size, new_alignment);
}

void *ki_fs_filename_allocator_t::realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	return mm_krealloc_in_place(ptr, new_size, new_alignment);
}

void ki_fs_filename_allocator_t::release(void *ptr, size_t size, size_t alignment) noexcept {
	mm_kfree(ptr);
}

int ki_fs_filename_allocator_identity;

void *ki_fs_filename_allocator_t::type_identity() const noexcept {
	return &ki_fs_filename_allocator_identity;
}

ki_fs_filename_allocator_t ki_fs_filename_allocator;

ki_fs_fnode_allocator_t::ki_fs_fnode_allocator_t() {
}

ki_fs_fnode_allocator_t::~ki_fs_fnode_allocator_t() {
}

size_t ki_fs_fnode_allocator_t::inc_ref() noexcept {
	return 0;
}

size_t ki_fs_fnode_allocator_t::dec_ref() noexcept {
	return 0;
}

void *ki_fs_fnode_allocator_t::alloc(size_t size, size_t alignment) noexcept {
	return mm_kalloc(size, alignment);
}

void *ki_fs_fnode_allocator_t::realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	// TODO: Implement mm_krealloc and rewrite this with it.
	return mm_krealloc(ptr, new_size, new_alignment);
}

void *ki_fs_fnode_allocator_t::realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	return mm_krealloc_in_place(ptr, new_size, new_alignment);
}

void ki_fs_fnode_allocator_t::release(void *ptr, size_t size, size_t alignment) noexcept {
	mm_kfree(ptr);
}

int ki_fs_fnode_allocator_identity;

void *ki_fs_fnode_allocator_t::type_identity() const noexcept {
	return &ki_fs_fnode_allocator_identity;
}

ki_fs_fnode_allocator_t ki_fs_fnode_allocator;

ki_fs_fcb_allocator_t::ki_fs_fcb_allocator_t() {
}

ki_fs_fcb_allocator_t::~ki_fs_fcb_allocator_t() {
}

size_t ki_fs_fcb_allocator_t::inc_ref() noexcept {
	return 0;
}

size_t ki_fs_fcb_allocator_t::dec_ref() noexcept {
	return 0;
}

void *ki_fs_fcb_allocator_t::alloc(size_t size, size_t alignment) noexcept {
	return mm_kalloc(size, alignment);
}

void *ki_fs_fcb_allocator_t::realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	// TODO: Implement mm_krealloc and rewrite this with it.
	return mm_krealloc(ptr, new_size, new_alignment);
}

void *ki_fs_fcb_allocator_t::realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	// TODO: Implement mm_krealloc and rewrite this with it.
	// return mm_krealloc(ptr, new_size, new_alignment);
	return nullptr;
}

void ki_fs_fcb_allocator_t::release(void *ptr, size_t size, size_t alignment) noexcept {
	mm_kfree(ptr);
}

int ki_fs_fcb_allocator_identity;

void *ki_fs_fcb_allocator_t::type_identity() const noexcept {
	return &ki_fs_fcb_allocator_identity;
}

ki_fs_fcb_allocator_t ki_fs_fcb_allocator;

_fs_fcb_t::_fs_fcb_t(fs_fnode_t *fnode) : fnode(fnode) {
}

_fs_fcb_t::~_fs_fcb_t() {
}

_fs_fnode_t::_fs_fnode_t(fs_filetype_t file_type)
	: file_type(file_type) {
}

_fs_fnode_t::~_fs_fnode_t() {
	ki_do_unname_fnode(this);
}

_fs_file_t::_fs_file_t() : _fs_fnode_t(FS_FILETYPE_FILE) {}

_fs_dir_t::_fs_dir_t(kfxx::Allocator *allocator) : _fs_fnode_t(FS_FILETYPE_DIR), subnodes(allocator) {}

PBOS_API void fs_set_fcb_exdata(fs_fcb_t *fcb, void *exdata) {
	fcb->exdata = exdata;
}

PBOS_NODISCARD PBOS_API void *fs_get_fcb_exdata(fs_fcb_t *fcb) {
	return fcb->exdata;
}

PBOS_NODISCARD PBOS_API km_result_t fs_alloc_file_fnode(fs_file_system_t *file_system, fs_fnode_t **file_out) {
	fs_file_t *ptr = kfxx::alloc_and_construct<fs_file_t>(&ki_fs_filename_allocator);

	if (!ptr)
		return KM_RESULT_NO_MEM;

	ptr->fs = file_system;

	fs_ref_fnode(ptr);

	*file_out = ptr;

	return KM_RESULT_OK;
}

PBOS_NODISCARD PBOS_API km_result_t fs_alloc_dir_fnode(fs_file_system_t *file_system, fs_fnode_t **file_out) {
	fs_dir_t *ptr = kfxx::alloc_and_construct<fs_dir_t>(&ki_fs_filename_allocator, &ki_fs_filename_allocator);

	if (!ptr)
		return KM_RESULT_NO_MEM;

	ptr->fs = file_system;

	fs_ref_fnode(ptr);

	*file_out = ptr;

	return KM_RESULT_OK;
}

PBOS_API void *fs_get_fnode_exdata(fs_fnode_t *file) {
	return file->exdata;
}

PBOS_API void fs_set_fnode_exdata(fs_fnode_t *file, void *exdata) {
	file->exdata = exdata;
}

PBOS_NODISCARD PBOS_API const char *fs_name_of_fnode(fs_fnode_t *file, size_t *len_out) {
	*len_out = file->filename_len;
	return file->filename;
}

PBOS_API void fs_unname_fnode(fs_fnode_t *file) {
	ki_do_unname_fnode(file);
}

PBOS_NODISCARD PBOS_API km_result_t fs_rename_fnode(fs_fnode_t *file, const char *name, size_t name_len) {
	if (file->parent)
		km_panic("Only detached file node can be renamed");

	return ki_do_rename_fnode(file, name, name_len);
}

PBOS_NODISCARD km_result_t ki_alloc_fcb(fs_fnode_t *file_in, fs_fcb_t **fcb_out) {
	fs_fcb_t *ptr = kfxx::alloc_and_construct<fs_fcb_t>(&ki_fs_filename_allocator, file_in);

	if (!ptr)
		return KM_RESULT_NO_MEM;

	ptr->fnode = file_in;

	*fcb_out = ptr;

	return KM_RESULT_OK;
}

void ki_do_unname_fnode(fs_fnode_t *file) {
	if (file->filename) {
		ki_fs_filename_allocator.release(file->filename, file->filename_len, alignof(char));
		file->filename = nullptr;
		file->filename_len = 0;
	}
}

km_result_t ki_do_rename_fnode(fs_fnode_t *file, const char *name, size_t name_len) {
	kd_dbgcheck(name_len, "The file name must not be empty");
	char *new_name;
	if (file->filename)
		new_name = (char *)ki_fs_filename_allocator.realloc(
			file->filename,
			file->filename_len, alignof(char),
			name_len, alignof(char));
	else
		new_name = (char *)ki_fs_filename_allocator.alloc(
			name_len,
			alignof(char));
	if (!new_name)
		return KM_RESULT_NO_MEM;
	memcpy(new_name, name, name_len);
	file->filename = new_name;
	file->filename_len = name_len;
	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_create_file(
	fs_fnode_t *parent,
	const char *filename,
	size_t filename_len,
	fs_fnode_t **file_out) {
	return parent->fs->ops.create_file(parent, filename, filename_len, file_out);
}

PBOS_API km_result_t fs_create_dir(
	fs_fnode_t *parent,
	const char *filename,
	size_t filename_len,
	fs_fnode_t **file_out) {
	return parent->fs->ops.create_dir(parent, filename, filename_len, file_out);
}

PBOS_NODISCARD PBOS_API km_result_t fs_create_fcb(
	fs_fnode_t *file,
	fs_fcb_t **fcb_out) {
}

PBOS_API km_result_t fs_mount_file(fs_fnode_t *parent, fs_fnode_t *file) {
	// The mount point should be a directory.
	if (file->file_type != FS_FILETYPE_DIR)
		return KM_RESULT_UNSUPPORTED_OPERATION;

	fs_dir_t *f = static_cast<fs_dir_t *>(file);

	// The mount point should be empty.
	if (f->subnodes.size())
		return KM_RESULT_DIR_NOT_EMPTY;

	if (!f->subnodes.shrink_buckets())
		return KM_RESULT_NO_MEM;

	fs_file_system_t *fs = parent->fs;

	KM_RETURN_IF_FAILED(fs->ops.premount(parent, file));

	{
		kfxx::ScopeGuard mount_fail_guard([fs, parent, file]() noexcept {
			fs->ops.mount_fail(parent, file);
		});

		if (!f->subnodes.insert(kfxx::StringView(file->filename, file->filename_len), +file))
			return KM_RESULT_NO_MEM;

		mount_fail_guard.release();
	}
	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_unmount_file(fs_fnode_t *file) {
	KM_RETURN_IF_FAILED(file->parent->fs->ops.unmount_cleanup(file));

	kd_assert(file->parent);

	fs_dir_t *parent = static_cast<fs_dir_t *>(file->parent);

	parent->subnodes.remove(kfxx::StringView(file->filename, file->filename_len));

	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_link_subnode(fs_fnode_t *parent, fs_fnode_t *file) {
	if (parent->file_type != FS_FILETYPE_DIR)
		return KM_RESULT_UNSUPPORTED_OPERATION;
	fs_dir_t *p = (fs_dir_t *)parent;

	if (p->subnodes.contains(kfxx::StringView(file->filename, file->filename_len)))
		return KM_RESULT_EXISTED;

	if (!p->subnodes.insert(kfxx::StringView(file->filename, file->filename_len), +file))
		return KM_RESULT_NO_MEM;

	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_child_of(fs_fnode_t *file, const char *filename, size_t filename_len, fs_fnode_t **file_out) {
	if (!file)
		return KM_RESULT_NOT_FOUND;
	switch (file->file_type) {
		case FS_FILETYPE_DIR: {
			fs_dir_t *f = static_cast<fs_dir_t *>(file);
			if (auto it = f->subnodes.find(kfxx::StringView(filename, filename_len)); it != f->subnodes.end()) {
				fs::FNodePtr node = it.value();
				*file_out = node.get();
				fs_ref_fnode(node.get());
			} else
				KM_RETURN_IF_FAILED(file->fs->ops.subnode(file, filename, filename_len, file_out));
			break;
		}
		case FS_FILETYPE_LINK:
			// TODO: Implement it.
			return KM_RESULT_UNSUPPORTED_OPERATION;
		default:
			return KM_RESULT_UNSUPPORTED_OPERATION;
	}

	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_resolve_path(fs_fnode_t *cur_dir, const char *path, size_t path_len, fs_fnode_t **file_out) {
	fs::FNodePtr file = cur_dir;
	km_result_t result;

	const char *i = path, *last_divider = path;
	fs::FNodePtr new_file;

	while (i - path < path_len) {
		switch (*i) {
			case '/': {
				size_t filename_len = i - last_divider;

				if (!filename_len) {
					if (last_divider == path) {
						new_file = fs_abs_root_dir;
					}
				} else {
					if (!file)
						return KM_RESULT_NOT_FOUND;
					if (KM_FAILED(result = fs_child_of(file.get(), last_divider, filename_len, new_file.get_addr())))
						return result;
				}

				last_divider = i + 1;
				file = new_file;
				break;
			}
			case '\0':
				goto end;
		}

		++i;
	}

end:;
	size_t filename_len = i - last_divider;
	if (filename_len) {
		if (!file)
			return KM_RESULT_NOT_FOUND;
		if (KM_FAILED(result = fs_child_of(file.get(), last_divider, filename_len, new_file.get_addr())))
			return result;
	}

	*file_out = new_file.release();
	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_open(fs_fnode_t *base_dir, const char *path, size_t path_len, fs_fcb_t **fcb_out) {
	fs::FNodePtr file;
	km_result_t result;

	if (KM_FAILED(result = fs_resolve_path(base_dir, path, path_len, file.get_addr_without_release())))
		return result;

	if (KM_FAILED(result = file->fs->ops.open(file.get(), fcb_out))) {
		return result;
	}

	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_close(fs_fcb_t *fcb) {
	fcb->fnode.reset();
	ki_destroy_fcb(fcb);
	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_read(fs_fcb_t *fcb, void *dest, size_t size, size_t off, size_t *bytes_read_out) {
	return fcb->fnode->fs->ops.read(fcb, (char *)dest, size, off, bytes_read_out);
}

PBOS_API km_result_t fs_write(fs_fcb_t *fcb, const void *src, size_t size, size_t off, size_t *bytes_written_out) {
	return fcb->fnode->fs->ops.write(fcb, src, size, off, bytes_written_out);
}

PBOS_API km_result_t fs_size(fs_fcb_t *fcb, size_t *size_out) {
	return fcb->fnode->fs->ops.size(fcb, size_out);
}

PBOS_API void fs_ref_fnode(fs_fnode_t *fnode) {
	kf_atomic_inc_size(&fnode->ref_count);
}

PBOS_API void fs_unref_fnode(fs_fnode_t *fnode) {
	if (!kf_atomic_dec_size(&fnode->ref_count))
		ki_destroy_fnode(fnode);
}

void ki_destroy_fnode(fs_fnode_t *fnode) {
	switch (fnode->file_type) {
		case FS_FILETYPE_FILE:
			fnode->fs->ops.destroy(fnode);
			kfxx::destroy_and_release<fs_file_t>(&ki_fs_fnode_allocator, static_cast<fs_file_t *>(fnode));
		case FS_FILETYPE_DIR:
			fnode->fs->ops.destroy(fnode);
			kfxx::destroy_and_release<fs_dir_t>(&ki_fs_fnode_allocator, static_cast<fs_dir_t *>(fnode));
		default:
			km_panic("Unknown file node type, panicking");
	}
}

void ki_destroy_fcb(fs_fcb_t *fcb) {
	if (fcb->fnode)
		km_panic("FCB is not closed before destroying");
	kfxx::destroy_and_release<fs_fcb_t>(&ki_fs_fcb_allocator, fcb);
}

PBOS_EXTERN_C_END
