#include <pbos/kd/logger.h>
#include <pbos/kh/mm/misc.hh>
#include <pbos/kh/initcar.hh>
#include <pbos/ki/fs/file.hh>
#include <pbos/ki/fs/fs.hh>

PBOS_EXTERN_C_BEGIN

constexpr kfxx::string_view INITCAR_DIR_FILENAME = kfxx::string_view("initcar");

void *kh_initcar_ptr = NULL;
void *kh_initcar_paddr = NULL;
size_t kh_initcar_file_size = 0;
bool kh_is_initcar_direct_mapped = true;
fs_fnode_t *kh_initcar_first_file = nullptr, *kh_initcar_last_file = nullptr;

fs_filesys_t *kh_initcar_fs = NULL;
fs_fnode_t *kh_initcar_dir = nullptr;

fs_filesys_ops_t kh_initcar_ops = {
	.subnode = kh_initcar_subnode,
	.offload = kh_initcar_offload,
	.create_file = kh_initcar_create_file,
	.create_dir = kh_initcar_create_dir,
	.open = kh_initcar_open,
	.close = kh_initcar_close,
	.seek = kh_initcar_seek,
	.read = kh_initcar_read,
	.write = kh_initcar_write,
	.pread = kh_initcar_pread,
	.pwrite = kh_initcar_pwrite,
	.ioctl = kh_initcar_ioctl,
	.size = kh_initcar_size,
	.enum_first_child_file = kh_initcar_enum_first_child_file,
	.enum_next_file = kh_initcar_enum_next_file,
	.destroy = kh_initcar_destroy,
	.premount = kh_initcar_premount,
	.mount_fail = kh_initcar_mount_fail,
	.unmount_cleanup = kh_initcar_unmount_cleanup,
	.destructor = kh_initcar_destructor
};

void kh_initcar_destroy(fs_fnode_t *file) {
	auto exdata = (kh_initcar_file_exdata *)fs_get_fnode_exdata(file);
	if (exdata)
		kfxx::destroy_and_release<kh_initcar_file_exdata>(kfxx::kernel_allocator(), exdata);
}

km_result_t kh_initcar_destructor() {
	// Because the root directory has taken the ownership,
	// we just need to unmount the directory and then it will be released automatically.
	if (KM_FAILED(fs_unmount_file(kh_initcar_dir)))
		km_panic("Error unounting the initcar directory");

	// TODO: Release the subnodes.

	mm_vmfree(mm_get_cur_context(), kh_initcar_ptr, kh_initcar_file_size);

	return KM_RESULT_OK;
}

km_result_t kh_initcar_premount(fs_fnode_t *parent, fs_fnode_t *file) {
	return KM_RESULT_OK;
}

km_result_t kh_initcar_postmount(fs_fnode_t *parent, fs_fnode_t *file) {
	return KM_RESULT_OK;
}

void kh_initcar_mount_fail(fs_fnode_t *parent, fs_fnode_t *file) {
}

km_result_t kh_initcar_unmount_cleanup(fs_fnode_t *file) {
	// TODO: Implement it.
	return KM_RESULT_OK;
}

void kh_initcar_init() {
	km_result_t result;

	if (!(kh_initcar_fs = fs_register_file_system("initcar", strlen("initcar"), &kh_initcar_ops, nullptr)))
		km_panic("Error registering initcar file system");

	dbg_printf("INITCAR range: %p-%p\n",
		kh_initcar_paddr,
		((const char *)kh_initcar_paddr) + kh_initcar_file_size);

	if ((!(kh_initcar_ptr = kh_get_direct_mmap(kh_initcar_paddr))) || (!kh_get_direct_mmap(((char *)kh_initcar_paddr) + kh_initcar_file_size))) {
		if (!(kh_initcar_ptr = mm_kvmalloc(mm_get_cur_context(), kh_initcar_file_size, MM_PAGE_READ, 0)))
			km_panic("Error allocating virtual memory space for INITCAR");
		if (KM_FAILED(result = mm_mmap(mm_get_cur_context(), kh_initcar_ptr, kh_initcar_paddr, kh_initcar_file_size, MM_PAGE_READ, 0)))
			km_panic("Error mapping INITCAR area");
	}

	size_t sz_left = kh_initcar_file_size;

	pbcar_metadata_t *md = (pbcar_metadata_t *)kh_initcar_ptr;
	if (md->magic[0] != PBCAR_MAGIC_0 ||
		md->magic[1] != PBCAR_MAGIC_1 ||
		md->magic[2] != PBCAR_MAGIC_2 ||
		md->magic[3] != PBCAR_MAGIC_3)
		km_panic("Invalid INITCAR magic, the file may be damaged or invalid");

	if (md->major_ver != 0 || md->minor_ver != 1)
		km_panic("Incompatible INITCAR version");

	if (md->flags & PBCAR_METADATA_BE)
		km_panic("Incompatible INITCAR byte-order");

	// Create file objects.
	const char *p_cur = ((const char *)kh_initcar_ptr) + sizeof(pbcar_metadata_t);
	const uint32_t kh_initcar_size = kh_initcar_file_size;

#define initcar_checksize(size)                                            \
	if (((p_cur - (const char *)kh_initcar_ptr) + size) > kh_initcar_file_size) \
		km_panic("Prematured end of file\n");

	if (KM_FAILED(result = fs_alloc_dir_fnode(kh_initcar_fs, &kh_initcar_dir)))
		km_panic("Error creating initcar directory, error code = %.0x", result);

	if (KM_FAILED(result = fs_rename_fnode(kh_initcar_dir, INITCAR_DIR_FILENAME.data(), INITCAR_DIR_FILENAME.size())))
		km_panic("Error creating initcar directory, error code = %.0x", result);

	{
		if (KM_FAILED(result = fs_link_subnode(fs_abs_root_dir, kh_initcar_dir)))
			km_panic("Error mounting initcar directory, error code = %.0x", result);
	}

	pbcar_fentry_t *fe;
	while (true) {
		initcar_checksize(sizeof(*fe));
		fe = (pbcar_fentry_t *)p_cur, p_cur += sizeof(*fe);

		if (fe->flags & PBCAR_FILE_FLAG_END)
			break;

		fs::fnode_ptr file;
		size_t filename_len = strlen(fe->filename);
		if (KM_FAILED(fs_alloc_file_fnode(kh_initcar_fs, &file)))
			km_panic("Error creating file object for initcar file: %s\n", fe->filename);
		if (KM_FAILED(fs_rename_fnode(file.get(), fe->filename, filename_len)))
			km_panic("Error creating file object for initcar file: %s\n", fe->filename);

		kh_initcar_file_exdata *exdata = kfxx::alloc_and_construct<kh_initcar_file_exdata>(kfxx::kernel_allocator());
		if (!exdata)
			km_panic("Error allocating extension data for INITCAR file: %s", fe->filename);
		exdata->ptr = p_cur;
		exdata->sz_total = fe->size;

		if(!kh_initcar_first_file) {
			kh_initcar_first_file = file.get();
		}
		if(kh_initcar_last_file) {
			exdata->prev = kh_initcar_last_file;
			((kh_initcar_file_exdata *)fs_get_fnode_exdata(kh_initcar_last_file))->next = kh_initcar_last_file;
		}
		kh_initcar_last_file = file.get();

		fs_set_fnode_exdata(file.get(), exdata);

		dbg_printf("initcar: Mounting file: %s\n", fe->filename);
		if (KM_FAILED(result = fs_link_subnode(kh_initcar_dir, file.get())))
			km_panic("Error mounting initcar file `%s', error code = %x\n", fe->filename, result);

		p_cur += fe->size;
	}
}

void kh_initcar_deinit() {
	// stub
}

PBOS_EXTERN_C_END
