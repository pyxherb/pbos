#include <pbos/km/logger.h>
#include <hal/x86_64/initcar.hh>
#include <pbos/kh/mm/misc.hh>
#include <pbos/ki/fs/file.hh>
#include <pbos/ki/fs/fs.hh>

PBOS_EXTERN_C_BEGIN

constexpr kfxx::string_view INITCAR_DIR_FILENAME = kfxx::string_view("initcar");

void *hn_initcar_ptr = NULL;
void *hn_initcar_paddr = NULL;
size_t hn_initcar_size = 0;
bool hn_is_initcar_direct_mapped = true;

fs_file_system_t *kh_initcar_fs = NULL;
fs_fnode_t *kh_initcar_dir;

fs_file_system_ops_t kh_initcar_ops = {
	.subnode = kh_initcar_subnode,
	.offload = kh_initcar_offload,
	.create_file = kh_initcar_create_file,
	.create_dir = kh_initcar_create_dir,
	.open = kh_initcar_open,
	.close = kh_initcar_close,
	.read = kh_initcar_read,
	.write = kh_initcar_write,
	.size = kh_initcar_size,
	.destroy = kh_initcar_destroy,
	.premount = kh_initcar_premount,
	.mount_fail = kh_initcar_mount_fail,
	.unmount_cleanup = kh_initcar_unmount_cleanup,
	.destructor = kh_initcar_destructor
};

void kh_initcar_destroy(fs_fnode_t *file) {
	auto exdata = (hn_initcar_file_exdata *)fs_get_fnode_exdata(file);
	if (exdata)
		kfxx::destroy_and_release<hn_initcar_file_exdata>(kfxx::kernel_allocator(), exdata);
}

km_result_t kh_initcar_destructor() {
	/*
	// Unmount all files.
	{
		fs_finddata_t finddata;
		fs_fnode_t *handle;
		fs_find_file(initcar_dir, &finddata, &handle);
		while (handle != OM_INVALID_HANDLE) {
			if (KM_FAILED(fs_unmount_file(handle)))
				km_panic("Error unmounting an initcar file");
			fs_find_next_file(&finddata, &handle);
		}
	}*/

	// Because the root directory has taken the ownership,
	// we just need to unmount the directory and then it will be released automatically.
	if (KM_FAILED(fs_unmount_file(kh_initcar_dir)))
		km_panic("Error unounting the initcar directory");

	mm_vmfree(mm_get_cur_context(), hn_initcar_ptr, hn_initcar_size);

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

	if (!(kh_initcar_fs = fs_register_file_system("initcar", strlen("initcar"), &kh_initcar_ops)))
		km_panic("Error registering initcar file system");

	kd_printf("INITCAR range: %p-%p\n",
		hn_initcar_paddr,
		((const char *)hn_initcar_paddr) + hn_initcar_size);

	if ((!(hn_initcar_ptr = kh_get_direct_mmap(hn_initcar_paddr))) || (!kh_get_direct_mmap(((char *)hn_initcar_paddr) + hn_initcar_size))) {
		if (!(hn_initcar_ptr = mm_kvmalloc(mm_get_cur_context(), hn_initcar_size, MM_PAGE_READ, 0)))
			km_panic("Error allocating virtual memory space for INITCAR");
		if (KM_FAILED(result = mm_mmap(mm_get_cur_context(), hn_initcar_ptr, hn_initcar_paddr, hn_initcar_size, MM_PAGE_READ, 0)))
			km_panic("Error mapping INITCAR area");
	}

	size_t sz_left = hn_initcar_size;

	pbcar_metadata_t *md = (pbcar_metadata_t *)hn_initcar_ptr;
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
	const char *p_cur = ((const char *)hn_initcar_ptr) + sizeof(pbcar_metadata_t);
	const uint32_t kh_initcar_size = hn_initcar_size;

#define initcar_checksize(size)                                            \
	if (((p_cur - (const char *)hn_initcar_ptr) + size) > kh_initcar_size) \
		km_panic("Prematured end of file\n");

	if (KM_FAILED(result = fs_alloc_dir_fnode(kh_initcar_fs, &kh_initcar_dir)))
		km_panic("Error creating initcar directory, error code = %.0x", result);

	if (KM_FAILED(result = fs_rename_fnode(kh_initcar_dir, INITCAR_DIR_FILENAME.data(), INITCAR_DIR_FILENAME.size())))
		km_panic("Error creating initcar directory, error code = %.0x", result);

	{
		// fs_fnode_t * root_handle;
		// if (KM_FAILED(fs_open("/", sizeof("/") - 1, &root_handle)))
		// km_panic("Error opening the root directory, error code = %.0x", result);
		if (KM_FAILED(result = fs_link_subnode(fs_abs_root_dir, kh_initcar_dir)))
			km_panic("Error mounting initcar directory, error code = %.0x", result);
	}

	pbcar_fentry_t *fe;
	while (true) {
		initcar_checksize(sizeof(*fe));
		fe = (pbcar_fentry_t *)p_cur, p_cur += sizeof(*fe);

		if (fe->flags & PBCAR_FILE_FLAG_END)
			break;

		kd_printf("File: %s\n", fe->filename);
		kd_printf("Size: %d\n", (int)fe->size);

		fs::fnode_ptr_t file;
		size_t filename_len = strlen(fe->filename);
		if (KM_FAILED(fs_alloc_file_fnode(kh_initcar_fs, &file)))
			km_panic("Error creating file object for initcar file: %s\n", fe->filename);
		if (KM_FAILED(fs_rename_fnode(file.get(), fe->filename, filename_len)))
			km_panic("Error creating file object for initcar file: %s\n", fe->filename);

		hn_initcar_file_exdata *exdata = kfxx::alloc_and_construct<hn_initcar_file_exdata>(kfxx::kernel_allocator());
		if (!exdata)
			km_panic("Error allocating extension data for INITCAR file: %s", fe->filename);
		exdata->ptr = p_cur;
		exdata->sz_total = fe->size;

		fs_set_fnode_exdata(file.get(), exdata);

		kd_printf("initcar: Mounting file: %s\n", fe->filename);
		if (KM_FAILED(result = fs_link_subnode(kh_initcar_dir, file.get())))
			km_panic("Error mounting initcar file `%s', error code = %x\n", fe->filename, result);

		p_cur += fe->size;
	}
}

void kh_initcar_deinit() {
	// stub
}

PBOS_EXTERN_C_END
