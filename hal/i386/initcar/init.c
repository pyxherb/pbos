#include <hal/i386/initcar.h>
#include <pbos/km/logger.h>
#include <pbos/kn/fs/file.h>
#include <pbos/kn/fs/fs.h>

void *initcar_ptr = NULL;

fs_filesys_t *initcar_fs = NULL;
om_handle_t initcar_dir_handle;

fs_fsops_t initcar_ops = {
	.open = initcar_open,
	.close = initcar_close,
	.read = initcar_read,
	.write = initcar_write,
	.size = initcar_size,
	.premount = initcar_premount,
	.postmount = initcar_postmount,
	.mountfail = initcar_mountfail,
	.unmount = initcar_unmount,
	.destructor = initcar_destructor
};

km_result_t initcar_destructor() {
	// Unmount all files.
	{
		fs_finddata_t finddata;
		om_handle_t handle;
		fs_find_file(initcar_dir_handle, &finddata, &handle);
		while (handle != OM_INVALID_HANDLE) {
			if (KM_FAILED(fs_unmount_file(handle)))
				km_panic("Error unmounting an initcar file");
			fs_find_next_file(&finddata, &handle);
		}
	}

	// Because the root directory has taken the ownership,
	// we just need to unmount the directory and then it will be released automatically.
	if (KM_FAILED(fs_unmount_file(initcar_dir_handle)))
		km_panic("Error unounting the initcar directory");

	mm_vmfree(mm_kernel_context, initcar_ptr, ARCH_KARGS_PTR->initcar_size);

	return KM_RESULT_OK;
}

km_result_t initcar_premount(fs_file_t *parent, fs_file_t *file_handle) {
	return KM_RESULT_OK;
}

km_result_t initcar_postmount(fs_file_t *parent, fs_file_t *file_handle) {
	return KM_RESULT_OK;
}

void initcar_mountfail(fs_file_t *parent, fs_file_t *file_handle) {
}

km_result_t initcar_unmount(fs_file_t *file_handle) {
	return KM_RESULT_OK;
}

void initcar_init() {
	km_result_t result;

	uuid_t uuid = INITCAR_UUID;
	if (!(initcar_fs = fs_register_filesys("initcar", &uuid, &initcar_ops)))
		km_panic("Error registering initcar file system");

	kdprintf("INITCAR range: %p-%p\n",
		ARCH_KARGS_PTR->initcar_ptr,
		((const char*)ARCH_KARGS_PTR->initcar_ptr) + ARCH_KARGS_PTR->initcar_size);

	size_t sz_left = ARCH_KARGS_PTR->initcar_size;

	if (!(initcar_ptr = mm_vmalloc(
			  mm_kernel_context,
			  (const void *)CRITICAL_VTOP + 1,
			  (const void *)UINTPTR_MAX,
			  ARCH_KARGS_PTR->initcar_size,
			  PAGE_READ,
			  0)))
		km_panic("Error allocating virtual memory space for INITCAR");

	mm_mmap(mm_kernel_context, initcar_ptr, ARCH_KARGS_PTR->initcar_ptr, ARCH_KARGS_PTR->initcar_size, PAGE_READ, 0);

	pbcar_metadata_t *md = initcar_ptr;
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
	const char *p_cur = ((const char*)initcar_ptr) + sizeof(pbcar_metadata_t);
	const uint32_t initcar_size = ARCH_KARGS_PTR->initcar_size;

#define initcar_checksize(size)                                      \
	if (((p_cur - (const char *)initcar_ptr) + size) > initcar_size) \
		km_panic("Prematured end of file\n");

	if (KM_FAILED(result = fs_create_dir(initcar_fs, "initcar", sizeof("initcar") - 1, 0, &initcar_dir_handle)))
		km_panic("Error creating initcar directory, error code = %.0x", result);

	{
		// om_handle_t root_handle;
		// if (KM_FAILED(fs_open("/", sizeof("/") - 1, &root_handle)))
		// km_panic("Error opening the root directory, error code = %.0x", result);
		if (KM_FAILED(result = fs_mount_file(fs_abs_root_dir, initcar_dir_handle)))
			km_panic("Error mounting initcar directory, error code = %.0x", result);
	}

	pbcar_fentry_t *fe;
	while (true) {
		initcar_checksize(sizeof(*fe));
		fe = (pbcar_fentry_t *)p_cur, p_cur += sizeof(*fe);

		if (fe->flags & PBCAR_FILE_FLAG_END)
			break;

		kdprintf("File: %s\n", fe->filename);
		kdprintf("Size: %d\n", (int)fe->size);

		om_handle_t file_handle;
		size_t filename_len = strlen(fe->filename);
		if (KM_FAILED(fs_create_file(
				initcar_fs,
				fe->filename, filename_len,
				sizeof(initcar_file_exdata_t) + filename_len,
				&file_handle)))
			km_panic("Error creating file object for initcar file: %s\n", fe->filename);

		fs_file_t *file;
		if (KM_FAILED(fs_deref_file_handle(file_handle, &file))) {
			km_panic("Error deferencing the file handle");
		}

		initcar_file_exdata_t *exdata = (initcar_file_exdata_t *)fs_file_exdata(file);

		exdata->ptr = p_cur;
		exdata->sz_total = fe->size;

		kdprintf("initcar: Mounting file: %s\n", fe->filename);
		if (KM_FAILED(result = fs_mount_file(initcar_dir_handle, file_handle)))
			km_panic("Error mounting initcar file `%s', error code = %x\n", fe->filename, result);

		p_cur += fe->size;
	}
}

void initcar_deinit() {
	mm_unmmap(mm_kernel_context, initcar_ptr, ARCH_KARGS_PTR->initcar_size, 0);
}
