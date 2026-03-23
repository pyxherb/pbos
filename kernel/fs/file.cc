#include <pbos/kf/hash.h>
#include <pbos/kf/string.h>
#include <pbos/mm/mm.h>
#include <pbos/kn/fs/file.hh>
#include <pbos/kn/fs/fs.hh>

PBOS_API kn_fs_filename_allocator_t::kn_fs_filename_allocator_t() {
}

PBOS_API kn_fs_filename_allocator_t::~kn_fs_filename_allocator_t() {
}

PBOS_API size_t kn_fs_filename_allocator_t::inc_ref() noexcept {
	return 0;
}

PBOS_API size_t kn_fs_filename_allocator_t::dec_ref() noexcept {
	return 0;
}

PBOS_API void *kn_fs_filename_allocator_t::alloc(size_t size, size_t alignment) noexcept {
	kd_assert(alignment >= alignof(max_align_t));
	return mm_kmalloc(size, alignof(max_align_t));
}

PBOS_API void *kn_fs_filename_allocator_t::realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	// TODO: Implement mm_krealloc and rewrite this with it.
	kd_assert(alignment >= alignof(max_align_t));
	return mm_krealloc(ptr, new_size, new_alignment);
}

PBOS_API void *kn_fs_filename_allocator_t::realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	// TODO: Implement mm_krealloc and rewrite this with it.
	kd_assert(alignment >= alignof(max_align_t));
	// return mm_krealloc(ptr, new_size, new_alignment);
	return nullptr;
}

PBOS_API void kn_fs_filename_allocator_t::release(void *ptr, size_t size, size_t alignment) noexcept {
	mm_kfree(ptr);
}

int kn_fs_filename_allocator_identity;

PBOS_API void *kn_fs_filename_allocator_t::type_identity() const noexcept {
	return &kn_fs_filename_allocator_identity;
}

kn_fs_filename_allocator_t kn_fs_filename_allocator;

PBOS_API kn_fs_fnode_allocator_t::kn_fs_fnode_allocator_t() {
}

PBOS_API kn_fs_fnode_allocator_t::~kn_fs_fnode_allocator_t() {
}

PBOS_API size_t kn_fs_fnode_allocator_t::inc_ref() noexcept {
	return 0;
}

PBOS_API size_t kn_fs_fnode_allocator_t::dec_ref() noexcept {
	return 0;
}

PBOS_API void *kn_fs_fnode_allocator_t::alloc(size_t size, size_t alignment) noexcept {
	kd_assert(alignment >= alignof(max_align_t));
	return mm_kmalloc(size, alignof(max_align_t));
}

PBOS_API void *kn_fs_fnode_allocator_t::realloc(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	// TODO: Implement mm_krealloc and rewrite this with it.
	kd_assert(alignment >= alignof(max_align_t));
	return mm_krealloc(ptr, new_size, new_alignment);
}

PBOS_API void *kn_fs_fnode_allocator_t::realloc_in_place(void *ptr, size_t size, size_t alignment, size_t new_size, size_t new_alignment) noexcept {
	// TODO: Implement mm_krealloc_in_place and rewrite this with it.
	kd_assert(alignment >= alignof(max_align_t));
	// return mm_krealloc(ptr, new_size, new_alignment);
	return nullptr;
}

PBOS_API void kn_fs_fnode_allocator_t::release(void *ptr, size_t size, size_t alignment) noexcept {
	mm_kfree(ptr);
}

int kn_fs_fnode_allocator_identity;

PBOS_API void *kn_fs_fnode_allocator_t::type_identity() const noexcept {
	return &kn_fs_fnode_allocator_identity;
}

kn_fs_fnode_allocator_t kn_fs_fnode_allocator;

_fs_fnode_t::_fs_fnode_t(fs_filetype_t file_type)
	: file_type(file_type) {
}

_fs_fnode_t::~_fs_fnode_t() {
	kn_do_unname_fnode(this);
}

_fs_file_t::_fs_file_t() : _fs_fnode_t(FS_FILETYPE_FILE) {}

_fs_dir_t::_fs_dir_t(kfxx::allocator_t *allocator) : _fs_fnode_t(FS_FILETYPE_DIR), children_map(allocator) {}

PBOS_NODISCARD km_result_t kn_alloc_file_fnode(fs_fnode_t **file_out) {
	fs_file_t *ptr = kfxx::alloc_and_construct<fs_file_t>(&kn_fs_filename_allocator);

	if (!ptr)
		return KM_RESULT_NO_MEM;

	fs_inc_fnode_ref(ptr);

	*file_out = ptr;

	return KM_RESULT_OK;
}

PBOS_NODISCARD km_result_t kn_alloc_dir_fnode(fs_fnode_t **file_out) {
	fs_file_t *ptr = kfxx::alloc_and_construct<fs_file_t>(&kn_fs_filename_allocator, &kn_fs_filename_allocator);

	if (!ptr)
		return KM_RESULT_NO_MEM;

	fs_inc_fnode_ref(ptr);

	*file_out = ptr;

	return KM_RESULT_OK;
}

PBOS_NODISCARD void kn_do_unname_fnode(fs_fnode_t *file_node) {
	if (file_node->filename) {
		kn_fs_filename_allocator.release(file_node->filename, file_node->filename_len, alignof(char));
		file_node->filename = nullptr;
		file_node->filename_len = 0;
	}
}

km_result_t kn_do_rename_fnode(fs_fnode_t *file_node, const char *name, size_t name_len) {
	kd_dbgcheck(name_len, "The file name must not be empty");
	char *new_name;
	if (file_node->filename)
		new_name = (char *)kn_fs_filename_allocator.realloc(
			file_node->filename,
			file_node->filename_len, alignof(char),
			name_len, alignof(char));
	else
		new_name = (char *)kn_fs_filename_allocator.alloc(
			file_node->filename_len,
			alignof(char));
	if (!new_name)
		return KM_RESULT_NO_MEM;
	file_node->filename = new_name;
	file_node->filename_len = name_len;
	return KM_RESULT_OK;
}

km_result_t fs_create_file(
	fs_fnode_t *parent,
	const char *filename,
	size_t filename_len,
	fs_fnode_t **file_out) {
	fs::fnode_ptr_t ptr;

	KM_RETURN_IF_FAILED(kn_alloc_file_fnode(ptr.get_addr()));

	return parent->fs->ops.create_file(parent, filename, filename_len, file_out);
}

km_result_t fs_create_dir(
	fs_fnode_t *parent,
	const char *filename,
	size_t filename_len,
	fs_fnode_t **file_out) {
	return parent->fs->ops.create_dir(parent, filename, filename_len, file_out);
}

km_result_t fs_mount_file(fs_fnode_t *parent, fs_fnode_t *file) {
	return parent->fs->ops.mount(parent, file);
}

km_result_t fs_unmount_file(fs_fnode_t *file) {
	return file->parent->fs->ops.unmount(file);
}

km_result_t fs_child_of(fs_fnode_t *file, const char *filename, size_t filename_len, fs_fnode_t **file_out) {
	switch (file->file_type) {
		case FS_FILETYPE_DIR: {
			fs_dir_t *f = static_cast<fs_dir_t *>(file);
			if (auto it = f->children_map.find(kfxx::string_view(filename, filename_len)); it != f->children_map.end()) {
				fs::fnode_ptr_t node = it.value();
				*file_out = node.get();
				fs_inc_fnode_ref(node.get());
			}
			else
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

km_result_t fs_resolve_path(fs_fnode_t *cur_dir, const char *path, size_t path_len, fs_fnode_t **file_out) {
	fs::fnode_ptr_t file = cur_dir ? fs_abs_root_dir : cur_dir;
	km_result_t result;

	const char *i = path, *last_divider = path;
	fs::fnode_ptr_t new_file;

	while (i - path < path_len) {
		switch (*i) {
			case '/': {
				size_t filename_len = i - last_divider;

				if (!filename_len) {
					if (last_divider == path) {
						new_file = fs_abs_root_dir;
					}
				} else if (KM_FAILED(result = fs_child_of(file.get(), last_divider, filename_len, new_file.get_addr())))
					return result;

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
		if (KM_FAILED(result = fs_child_of(file.get(), last_divider, filename_len, new_file.get_addr())))
			return result;
	}

	*file_out = new_file.release();
	return KM_RESULT_OK;
}

km_result_t fs_open(fs_fnode_t *base_dir, const char *path, size_t path_len, fs_fcb_t **fcb_out) {
	fs_fnode_t *file;
	km_result_t result;

	if (KM_FAILED(result = fs_resolve_path(base_dir, path, path_len, &file)))
		return result;

	if (KM_FAILED(result = file->fs->ops.open(file, fcb_out))) {
		fs_inc_fnode_ref(file);
		return result;
	}

	fs_dec_fnode_ref(file);

	return KM_RESULT_OK;
}

km_result_t fs_close(fs_fcb_t *fcb) {
	return fcb->file->fs->ops.close(fcb);
}

km_result_t fs_read(fs_fcb_t *fcb, void *dest, size_t size, size_t off, size_t *bytes_read_out) {
	return fcb->file->fs->ops.read(fcb, (char *)dest, size, off, bytes_read_out);
}

km_result_t fs_write(fs_fcb_t *fcb, const char *src, size_t size, size_t off, size_t *bytes_written_out) {
	return fcb->file->fs->ops.write(fcb, src, size, off, bytes_written_out);
}

km_result_t fs_size(fs_fcb_t *fcb, size_t *size_out) {
	return fcb->file->fs->ops.size(fcb, size_out);
}

void fs_inc_fnode_ref(fs_fnode_t *fnode) {
	// TODO: Use atomic version of this:
	++fnode->ref_num;
}

void fs_dec_fnode_ref(fs_fnode_t *fnode) {
	if (!--fnode->ref_num)
		kn_destroy_fnode(fnode);
}

void kn_destroy_fnode(fs_fnode_t *fnode) {
	// TODO: Implement this.
	switch (fnode->file_type) {
		case FS_FILETYPE_FILE:
			kfxx::destroy_and_release<fs_file_t>(&kn_fs_fnode_allocator, static_cast<fs_file_t *>(fnode));
		case FS_FILETYPE_DIR:
			kfxx::destroy_and_release<fs_dir_t>(&kn_fs_fnode_allocator, static_cast<fs_dir_t *>(fnode));
	}
}
