#include <pbos/kf/atomic.h>
#include <pbos/kf/hash.h>
#include <pbos/kf/string.h>
#include <pbos/mm/mm.h>
#include <pbos/ki/fs/file.hh>
#include <pbos/ki/fs/fs.hh>
#include <pbos/ki/io/dispatch.hh>
#include <pbos/ki/km/symbol.hh>

PBOS_EXTERN_C_BEGIN

PBOS_API fs_fnode_t *fs_get_file_of_fcb(fs_fcb_t *fcb) {
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

void *ki_fs_filename_allocator_t::realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size) noexcept {
	return mm_krealloc_in_place(ptr, new_size);
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

void *ki_fs_fnode_allocator_t::realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size) noexcept {
	return mm_krealloc_in_place(ptr, new_size);
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

void *ki_fs_fcb_allocator_t::realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size) noexcept {
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

_fs_fnode_t::_fs_fnode_t(fs_fnode_type_t file_type)
	: fnode_type(file_type) {
}

_fs_fnode_t::~_fs_fnode_t() {
	ki_do_unname_fnode(this);
}

_fs_file_t::_fs_file_t() : _fs_fnode_t(FS_FNODE_TYPE_FILE) {}

fs_dir_t::fs_dir_t(kfxx::allocator_t *allocator) : _fs_fnode_t(FS_FNODE_TYPE_DIR), subnodes(allocator) {}

PBOS_API void fs_set_fcb_exdata(fs_fcb_t *fcb, void *exdata) {
	fcb->exdata = exdata;
}

PBOS_NODISCARD PBOS_API void *fs_get_fcb_exdata(fs_fcb_t *fcb) {
	return fcb->exdata;
}

PBOS_NODISCARD PBOS_API km_result_t fs_alloc_file_fnode(fs_filesys_t *file_system, fs_fnode_t **file_out) {
	fs_file_t *ptr = kfxx::alloc_and_construct<fs_file_t>(&ki_fs_filename_allocator);

	if (!ptr)
		return KM_RESULT_NO_MEM;

	ptr->fs = file_system;

	fs_ref_fnode(ptr);

	*file_out = ptr;

	return KM_RESULT_OK;
}

PBOS_NODISCARD PBOS_API km_result_t fs_alloc_dir_fnode(fs_filesys_t *file_system, fs_fnode_t **file_out) {
	fs_dir_t *ptr = kfxx::alloc_and_construct<fs_dir_t>(&ki_fs_filename_allocator, &ki_fs_filename_allocator);

	if (!ptr)
		return KM_RESULT_NO_MEM;

	ptr->fs = file_system;

	fs_ref_fnode(ptr);

	*file_out = ptr;

	return KM_RESULT_OK;
}

PBOS_NODISCARD PBOS_API km_result_t fs_alloc_device_fnode(fs_filesys_t *file_system, fs_fnode_t **file_out) {
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

PBOS_NODISCARD PBOS_API const char *fs_get_fnode_name(fs_fnode_t *file, size_t *len_out) {
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
		ki_fs_filename_allocator.release(file->filename, file->filename_len + 1, alignof(char));
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
			file->filename_len + 1, alignof(char),
			name_len + 1, alignof(char));
	else
		new_name = (char *)ki_fs_filename_allocator.alloc(
			name_len + 1,
			alignof(char));
	if (!new_name)
		return KM_RESULT_NO_MEM;
	memcpy(new_name, name, name_len);
	new_name[name_len] = '\0';
	file->filename = new_name;
	file->filename_len = name_len;
	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_create_file(
	fs_fnode_t *parent,
	const char *filename,
	size_t filename_len,
	fs_fnode_t **file_out) {
	if (!parent->fs->ops.create_dir)
		return KM_RESULT_UNSUPPORTED_OPERATION;
	io_dispatch_context_t dispatch_context;
	io_init_dispatch_context(&dispatch_context);

	KM_RETURN_IF_FAILED(
		parent->fs->ops.create_file(
			&dispatch_context,
			parent,
			filename,
			filename_len,
			file_out));

	KM_RETURN_IF_FAILED(ki_poll_ctbs(&dispatch_context));

	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_create_dir(
	fs_fnode_t *parent,
	const char *filename,
	size_t filename_len,
	fs_fnode_t **file_out) {
	if (!parent->fs->ops.create_dir)
		return KM_RESULT_UNSUPPORTED_OPERATION;
	io_dispatch_context_t dispatch_context;
	io_init_dispatch_context(&dispatch_context);
	KM_RETURN_IF_FAILED(parent->fs->ops.create_dir(&dispatch_context, parent, filename, filename_len, file_out));

	KM_RETURN_IF_FAILED(ki_poll_ctbs(&dispatch_context));

	return KM_RESULT_OK;
}

PBOS_NODISCARD PBOS_API km_result_t fs_create_fcb(
	fs_fnode_t *file,
	fs_fcb_t **fcb_out) {
	return ki_alloc_fcb(file, fcb_out);
}

PBOS_NODISCARD PBOS_API km_result_t fs_remove(
	fs_fnode_t *fnode) {
	if (!fnode->fs->ops.remove)
		return KM_RESULT_UNSUPPORTED_OPERATION;
	io_dispatch_context_t dispatch_context;
	io_init_dispatch_context(&dispatch_context);
	KM_RETURN_IF_FAILED(fnode->fs->ops.remove(&dispatch_context, fnode));

	KM_RETURN_IF_FAILED(ki_poll_ctbs(&dispatch_context));

	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_mount(fs_fnode_t *parent, fs_fnode_t *file) {
	// The mount point should be a directory.
	if (parent->fnode_type != FS_FNODE_TYPE_DIR)
		return KM_RESULT_UNSUPPORTED_OPERATION;

	if (file->fnode_type != FS_FNODE_TYPE_DIR)
		return KM_RESULT_UNSUPPORTED_OPERATION;

	fs_dir_t *p = static_cast<fs_dir_t *>(parent);

	// The mount point should be empty.
	if (p->subnodes.size())
		return KM_RESULT_DIR_NOT_EMPTY;

	if (p->mounted_dir)
		return KM_RESULT_EXISTED;

	fs::fnode_write_lock_guard g(parent);

	if (!p->subnodes.shrink_buckets())
		return KM_RESULT_NO_MEM;

	fs_filesys_t *fs = parent->fs;

	KM_RETURN_IF_FAILED(fs->ops.premount(parent, file));

	{
		p->mount_backup.fs = p->fs;
		p->mounted_dir = file;
	}

	file->parent = p;
	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_unmount(fs_fnode_t *file) {
	if (file->fnode_type != FS_FNODE_TYPE_DIR)
		return KM_RESULT_UNSUPPORTED_OPERATION;

	fs_dir_t *parent = static_cast<fs_dir_t *>(file->parent);

	if (!parent->mounted_dir)
		return KM_RESULT_UNSUPPORTED_OPERATION;

	KM_RETURN_IF_FAILED(file->parent->fs->ops.unmount_cleanup(file));

	parent->mounted_dir = nullptr;
	parent->fs = parent->mount_backup.fs;

	file->parent = nullptr;

	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_link_subnode(fs_fnode_t *parent, fs_fnode_t *file) {
	if (parent->fnode_type != FS_FNODE_TYPE_DIR)
		return KM_RESULT_UNSUPPORTED_OPERATION;
	fs_dir_t *p = (fs_dir_t *)parent;

	auto &subnodes = p->mounted_dir ? ((fs_dir_t *)p->mounted_dir.get())->subnodes : p->subnodes;

	if (subnodes.contains(kfxx::string_view(file->filename, file->filename_len)))
		return KM_RESULT_EXISTED;

	if (!subnodes.insert(kfxx::string_view(file->filename, file->filename_len), +file))
		return KM_RESULT_NO_MEM;

	return KM_RESULT_OK;
}
PBOS_NODISCARD PBOS_API km_result_t fs_unlink_subnode(fs_fnode_t *file) {
	if (!file->parent)
		return KM_RESULT_INVALID_ARGS;

	fs_dir_t *p = (fs_dir_t *)file->parent;

	fs::fnode_read_lock_guard g(p);

	kfxx::string_view name_view(file->filename, file->filename_len);
	kd_dbgcheck(p->subnodes.contains(name_view), "The parent fnode does not contain the child fnode, please report this bug");

	p->subnodes.remove(name_view);

	return KM_RESULT_OK;
}

PBOS_API fs_fnode_t *fs_parent_of(fs_fnode_t *file) {
	fs::fnode_read_lock_guard g(file);
	if (file->parent) {
		fs_ref_fnode(file->parent);
		return file->parent;
	}
	return nullptr;
}

PBOS_API km_result_t fs_child_of(fs_fnode_t *file, const char *filename, size_t filename_len, fs_fnode_t **file_out) {
	if (!file)
		return KM_RESULT_NOT_FOUND;
	switch (file->fnode_type) {
		case FS_FNODE_TYPE_DIR: {
			fs_dir_t *p = static_cast<fs_dir_t *>(file);
			auto &subnodes = p->mounted_dir ? ((fs_dir_t *)p->mounted_dir.get())->subnodes : p->subnodes;
			if (auto it = subnodes.find(kfxx::string_view(filename, filename_len)); it != subnodes.end()) {
				fs::fnode_ptr node = it.value();
				*file_out = node.get();
				fs_ref_fnode(node.get());
			} else {
				if (!p->fs->ops.subnode)
					return KM_RESULT_UNSUPPORTED_OPERATION;
				KM_RETURN_IF_FAILED(p->fs->ops.subnode(file, filename, filename_len, file_out));
			}
			break;
		}
		case FS_FNODE_TYPE_LINK:
			// TODO: Implement it.
			return KM_RESULT_UNSUPPORTED_OPERATION;
		default:
			return KM_RESULT_UNSUPPORTED_OPERATION;
	}

	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_resolve_path(fs_fnode_t *cur_dir, const char *path, size_t path_len, fs_fnode_t **file_out) {
	fs::fnode_ptr file = cur_dir;
	km_result_t result;

	const char *i = path, *last_divider = path;
	fs::fnode_ptr new_file;

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

PBOS_NODISCARD km_result_t fs_enum_first_child_file(fs_fnode_t *dir, fs_fnode_t **first_file_out) {
	if (!dir->fs->ops.enum_first_child_file)
		return KM_RESULT_UNSUPPORTED_OPERATION;
	return dir->fs->ops.enum_first_child_file(dir, first_file_out);
}

PBOS_NODISCARD km_result_t fs_enum_next_file(fs_fnode_t *cur_file, fs_fnode_t **next_file_out) {
	if (!cur_file->fs->ops.enum_next_file)
		return KM_RESULT_UNSUPPORTED_OPERATION;
	return cur_file->fs->ops.enum_next_file(cur_file, next_file_out);
}

PBOS_API km_result_t fs_open(fs_fnode_t *base_dir, const char *path, size_t path_len, fs_fcb_t **fcb_out, fs_open_flags_t flags) {
	fs::fnode_ptr file;
	km_result_t result;

	if (KM_FAILED(result = fs_resolve_path(base_dir, path, path_len, file.get_addr_without_release())))
		return result;

	if (!file->fs->ops.open)
		return KM_RESULT_UNSUPPORTED_OPERATION;

	if (KM_FAILED(result = file->fs->ops.open(file.get(), fcb_out, flags))) {
		return result;
	}

	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_close(fs_fcb_t *fcb) {
	// Because the FCB is about to be destroyed, so we don't need to care about
	// if the FCB's semaphore will be unlocked.
	fcb->rw_semaphore.write_lock();

	fcb->fnode.reset();
	ki_destroy_fcb(fcb);
	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_seek(fs_fcb_t *fcb, long off, fs_seek_mode_t mode) {
	fs::fcb_write_lock_guard g(fcb);

	if (!fcb->fnode->fs->ops.seek)
		return KM_RESULT_UNSUPPORTED_OPERATION;

	io_dispatch_context_t dispatch_context;
	io_init_dispatch_context(&dispatch_context);

	KM_RETURN_IF_FAILED(fcb->fnode->fs->ops.seek(&dispatch_context, fcb, off, mode));

	KM_RETURN_IF_FAILED(ki_poll_ctbs(&dispatch_context));
	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_read(fs_fcb_t *fcb, void *dest, size_t size, size_t *bytes_read_out) {
	fs::fcb_read_lock_guard g(fcb);

	if (!fcb->fnode->fs->ops.read)
		return KM_RESULT_UNSUPPORTED_OPERATION;

	io_dispatch_context_t dispatch_context;
	io_init_dispatch_context(&dispatch_context);

	KM_RETURN_IF_FAILED(fcb->fnode->fs->ops.read(&dispatch_context, fcb, (char *)dest, size, bytes_read_out));

	KM_RETURN_IF_FAILED(ki_poll_ctbs(&dispatch_context));
	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_write(fs_fcb_t *fcb, const void *src, size_t size, size_t *bytes_written_out) {
	fs::fcb_write_lock_guard g(fcb);

	if (!fcb->fnode->fs->ops.write)
		return KM_RESULT_UNSUPPORTED_OPERATION;

	io_dispatch_context_t dispatch_context;
	io_init_dispatch_context(&dispatch_context);

	KM_RETURN_IF_FAILED(fcb->fnode->fs->ops.write(&dispatch_context, fcb, (char *)src, size, bytes_written_out));

	KM_RETURN_IF_FAILED(ki_poll_ctbs(&dispatch_context));
	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_pread(fs_fcb_t *fcb, void *dest, size_t size, size_t off, size_t *bytes_read_out) {
	fs::fcb_read_lock_guard g(fcb);

	if (!fcb->fnode->fs->ops.pread)
		return KM_RESULT_UNSUPPORTED_OPERATION;

	io_dispatch_context_t dispatch_context;
	io_init_dispatch_context(&dispatch_context);

	KM_RETURN_IF_FAILED(fcb->fnode->fs->ops.pread(&dispatch_context, fcb, (char *)dest, size, off, bytes_read_out));

	KM_RETURN_IF_FAILED(ki_poll_ctbs(&dispatch_context));
	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_pwrite(fs_fcb_t *fcb, const void *src, size_t size, size_t off, size_t *bytes_written_out) {
	fs::fcb_write_lock_guard g(fcb);

	if (!fcb->fnode->fs->ops.pwrite)
		return KM_RESULT_UNSUPPORTED_OPERATION;

	io_dispatch_context_t dispatch_context;
	io_init_dispatch_context(&dispatch_context);

	KM_RETURN_IF_FAILED(fcb->fnode->fs->ops.pwrite(&dispatch_context, fcb, (char *)src, size, off, bytes_written_out));

	KM_RETURN_IF_FAILED(ki_poll_ctbs(&dispatch_context));
	return KM_RESULT_OK;
}

PBOS_API km_result_t fs_size(fs_fcb_t *fcb, size_t *size_out) {
	fs::fcb_read_lock_guard g(fcb);

	if (!fcb->fnode->fs->ops.size)
		return KM_RESULT_UNSUPPORTED_OPERATION;

	io_dispatch_context_t dispatch_context;
	io_init_dispatch_context(&dispatch_context);

	KM_RETURN_IF_FAILED(fcb->fnode->fs->ops.size(&dispatch_context, fcb, size_out));

	KM_RETURN_IF_FAILED(ki_poll_ctbs(&dispatch_context));
	return KM_RESULT_OK;
}

PBOS_API void fs_ref_fnode(fs_fnode_t *fnode) {
	kf_atomic_inc_size(&fnode->ref_count);
}

PBOS_API void fs_unref_fnode(fs_fnode_t *fnode) {
	if (!kf_atomic_dec_size(&fnode->ref_count))
		ki_destroy_fnode(fnode);
}

PBOS_API void fs_read_lock_fnode(fs_fnode_t *fnode) {
	fnode->rw_semaphore.read_lock();
}

PBOS_API void fs_read_unlock_fnode(fs_fnode_t *fnode) {
	fnode->rw_semaphore.read_unlock();
}

PBOS_API void fs_write_lock_fnode(fs_fnode_t *fnode) {
	fnode->rw_semaphore.write_lock();
}

PBOS_API void fs_write_unlock_fnode(fs_fnode_t *fnode) {
	fnode->rw_semaphore.write_unlock();
}

PBOS_API void fs_read_lock_fcb(fs_fcb_t *fcb) {
	fcb->rw_semaphore.read_lock();
}

PBOS_API void fs_read_unlock_fcb(fs_fcb_t *fcb) {
	fcb->rw_semaphore.read_unlock();
}

PBOS_API void fs_write_lock_fcb(fs_fcb_t *fcb) {
	fcb->rw_semaphore.write_lock();
}

PBOS_API void fs_write_unlock_fcb(fs_fcb_t *fcb) {
	fcb->rw_semaphore.write_unlock();
}

PBOS_API fs_fnode_type_t fs_get_fnode_type(fs_fnode_t *fnode) {
	return fnode->fnode_type;
}

PBOS_API fs_filesys_t *fs_filesys_of_fnode(fs_fnode_t *fnode) {
	return fnode->fs;
}

void ki_destroy_fnode(fs_fnode_t *fnode) {
	switch (fnode->fnode_type) {
		case FS_FNODE_TYPE_FILE:
			fnode->fs->ops.destroy(fnode);
			kfxx::destroy_and_release<fs_file_t>(&ki_fs_fnode_allocator, static_cast<fs_file_t *>(fnode));
		case FS_FNODE_TYPE_DIR:
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
