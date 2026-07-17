#include <pbos/kd/logger.h>
#include <pbos/kh/initcar.hh>
#include <pbos/kh/mm/misc.hh>
#include <pbos/ki/fs/file.hh>
#include <pbos/ki/fs/fs.hh>

PBOS_EXTERN_C_BEGIN

constexpr kfxx::string_view INITCAR_DIR_FILENAME = kfxx::string_view("initcar");

void *ki_initcar_ptr = NULL;
void *ki_initcar_paddr = NULL;
size_t ki_initcar_file_size = 0;
bool kh_is_initcar_direct_mapped = true;
fs_fnode_t *ki_initcar_first_file = nullptr, *ki_initcar_last_file = nullptr;

fs_filesys_t *ki_initcar_filesys = NULL;
fs_fnode_t *ki_initcar_dir = nullptr;

fs_filesys_ops_t ki_initcar_ops = {
	.subnode = ki_initcar_subnode,
	.offload = ki_initcar_offload,
	.create_file = ki_initcar_create_file,
	.create_dir = ki_initcar_create_dir,
	.open = ki_initcar_open,
	.close_cleanup = ki_initcar_close_cleanup,
	.seek = ki_initcar_seek,
	.read = ki_initcar_read,
	.write = ki_initcar_write,
	.pread = ki_initcar_pread,
	.pwrite = ki_initcar_pwrite,
	.ioctl = ki_initcar_ioctl,
	.size = ki_initcar_size,
	.enum_first_child_file = ki_initcar_enum_first_child_file,
	.enum_next_file = ki_initcar_enum_next_file,
	.destroy = ki_initcar_destroy,
	.premount = ki_initcar_premount,
	.unmount_cleanup = ki_initcar_unmount_cleanup,
	.destructor = ki_initcar_destructor
};

void ki_initcar_destroy(fs_fnode_t *file) {
	auto exdata = (ki_initcar_file_exdata *)fs_get_fnode_exdata(file);
	if (exdata)
		kfxx::destroy_and_release<ki_initcar_file_exdata>(kfxx::kernel_allocator(), exdata);
}

km_result_t ki_initcar_destructor() {
	// Because the root directory has taken the ownership,
	// we just need to unmount the directory and then it will be released automatically.
	if (KM_FAILED(fs_unmount(ki_initcar_dir)))
		km_panic("Error unounting the initcar directory");

	// TODO: Release the subnodes.

	mm_vmfree(mm_get_cur_context(), ki_initcar_ptr, ki_initcar_file_size);

	return KM_RESULT_OK;
}

km_result_t ki_initcar_premount(fs_fnode_t *parent, fs_fnode_t *file) {
	return KM_RESULT_OK;
}

km_result_t ki_initcar_postmount(fs_fnode_t *parent, fs_fnode_t *file) {
	return KM_RESULT_OK;
}

km_result_t ki_initcar_unmount_cleanup(fs_fnode_t *file) {
	// TODO: Implement it.
	return KM_RESULT_OK;
}

void ki_initcar_init() {
	km_result_t result;

	if (!(ki_initcar_filesys = fs_register_file_system("initcar", strlen("initcar"), &ki_initcar_ops, nullptr)))
		km_panic("Error registering initcar file system");

	kd_printf("INITCAR range: %p-%p\n",
		ki_initcar_paddr,
		((const char *)ki_initcar_paddr) + ki_initcar_file_size);

	if ((!(ki_initcar_ptr = kh_get_direct_mmap(ki_initcar_paddr))) || (!kh_get_direct_mmap(((char *)ki_initcar_paddr) + ki_initcar_file_size))) {
		if (!(ki_initcar_ptr = mm_kvmalloc(mm_get_cur_context(), ki_initcar_file_size, MM_PAGE_READ, 0)))
			km_panic("Error allocating virtual memory space for INITCAR");
		if (KM_FAILED(result = mm_mmap(mm_get_cur_context(), ki_initcar_ptr, ki_initcar_paddr, ki_initcar_file_size, MM_PAGE_READ, 0)))
			km_panic("Error mapping INITCAR area");
	}

	size_t sz_left = ki_initcar_file_size;

	pbcar_metadata_t *md = (pbcar_metadata_t *)ki_initcar_ptr;
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
	const char *p_cur = ((const char *)ki_initcar_ptr) + sizeof(pbcar_metadata_t);
	const uint32_t ki_initcar_size = ki_initcar_file_size;

#define initcar_checksize(size)                                                 \
	if (((p_cur - (const char *)ki_initcar_ptr) + size) > ki_initcar_file_size) \
		km_panic("Prematured end of file\n");

	fs::fnode_ptr mount_point;
	{
		if (KM_FAILED(result = fs_create_child_dir(fs_abs_root_dir, INITCAR_DIR_FILENAME.data(), INITCAR_DIR_FILENAME.size(), &mount_point)))
			km_panic("Error creating initcar mount point directory, error code = %.0x", result);
	}

	if (KM_FAILED(result = fs_alloc_dir_fnode(ki_initcar_filesys, &ki_initcar_dir)))
		km_panic("Error creating initcar directory, error code = %.0x", result);

	if (KM_FAILED(result = fs_mount(mount_point.get(), ki_initcar_dir)))
		km_panic("Error mounting initcar directory, error code = %x\n", result);

	pbcar_fentry_t *fe;
	while (true) {
		initcar_checksize(sizeof(*fe));
		fe = (pbcar_fentry_t *)p_cur, p_cur += sizeof(*fe);

		if (fe->flags & PBCAR_FILE_FLAG_END)
			break;

		fs::fnode_ptr file;
		size_t filename_len = strlen(fe->filename);
		if (KM_FAILED(fs_alloc_file_fnode(ki_initcar_filesys, &file)))
			km_panic("Error creating file object for initcar file: %s\n", fe->filename);
		if (KM_FAILED(fs_rename_fnode(file.get(), fe->filename, filename_len)))
			km_panic("Error creating file object for initcar file: %s\n", fe->filename);

		ki_initcar_file_exdata *exdata = kfxx::alloc_and_construct<ki_initcar_file_exdata>(kfxx::kernel_allocator());
		if (!exdata)
			km_panic("Error allocating extension data for INITCAR file: %s", fe->filename);
		exdata->ptr = p_cur;
		exdata->sz_total = fe->size;

		if (!ki_initcar_first_file) {
			ki_initcar_first_file = file.get();
		}
		if (ki_initcar_last_file) {
			exdata->prev = ki_initcar_last_file;
			((ki_initcar_file_exdata *)fs_get_fnode_exdata(ki_initcar_last_file))->next = ki_initcar_last_file;
		}
		ki_initcar_last_file = file.get();

		fs_set_fnode_exdata(file.get(), exdata);

		kd_printf("initcar: Mounting file: %s\n", fe->filename);
		if (KM_FAILED(result = fs_link_subnode(ki_initcar_dir, file.get())))
			km_panic("Error mounting initcar file `%s', error code = %x\n", fe->filename, result);

		p_cur += fe->size;
	}
}

void ki_initcar_deinit() {
	// stub
}

PBOS_EXTERN_C_END
