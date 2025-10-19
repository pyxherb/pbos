#include <hal/i386/initcar.h>
#include <pbos/km/logger.h>
#include <pbos/kn/fs/file.hh>
#include <pbos/kn/fs/fs.hh>

PBOS_EXTERN_C_BEGIN

void *initcar_ptr = NULL;

fs_filesys_t *initcar_fs = NULL;
fs_file_t *initcar_dir;

fs_fsops_t initcar_ops = {
	.subnode = initcar_subnode,
	.offload = initcar_offload,
	.create_file = initcar_create_file,
	.create_dir = initcar_create_dir,
	.open = initcar_open,
	.close = initcar_close,
	.read = initcar_read,
	.write = initcar_write,
	.size = initcar_size,
	.mount = initcar_mount,
	.premount = initcar_premount,
	.postmount = initcar_postmount,
	.mountfail = initcar_mountfail,
	.unmount = initcar_unmount,
	.destructor = initcar_destructor
};

km_result_t initcar_destructor() {
	/*
	// Unmount all files.
	{
		fs_finddata_t finddata;
		fs_file_t *handle;
		fs_find_file(initcar_dir, &finddata, &handle);
		while (handle != OM_INVALID_HANDLE) {
			if (KM_FAILED(fs_unmount_file(handle)))
				km_panic("Error unmounting an initcar file");
			fs_find_next_file(&finddata, &handle);
		}
	}*/

	// Because the root directory has taken the ownership,
	// we just need to unmount the directory and then it will be released automatically.
	if (KM_FAILED(fs_unmount_file(initcar_dir)))
		km_panic("Error unounting the initcar directory");

	mm_vmfree(mm_kernel_context, initcar_ptr, ARCH_KARGS_PTR->initcar_size);

	return KM_RESULT_OK;
}

km_result_t initcar_mount(fs_file_t *parent, fs_file_t *file) {
	if (parent == initcar_dir) {
		initcar_dir_entry_t *dir_entry = (initcar_dir_entry_t*)mm_kmalloc(sizeof(initcar_dir_entry_t), alignof(initcar_dir_entry_t));
		if (!dir_entry)
			return KM_MAKEERROR(KM_RESULT_NO_MEM);
		memset(dir_entry, 0, sizeof(*dir_entry));
		dir_entry->name = file->filename;
		dir_entry->name_len = file->filename_len;
		dir_entry->file = file;
		km_result_t result = kf_hashmap_insert(&((initcar_dir_exdata_t *)parent->exdata)->children, &dir_entry->node_header);
		kd_assert(KM_SUCCEEDED(result));
		return KM_RESULT_OK;
	}
	return KM_MAKEERROR(KM_RESULT_UNSUPPORTED_OPERATION);
}

km_result_t initcar_premount(fs_file_t *parent, fs_file_t *file) {
	return KM_RESULT_OK;
}

km_result_t initcar_postmount(fs_file_t *parent, fs_file_t *file) {
	return KM_RESULT_OK;
}

void initcar_mountfail(fs_file_t *parent, fs_file_t *file) {
}

km_result_t initcar_unmount(fs_file_t *file) {
	// TODO: Implement it.
	return KM_RESULT_OK;
}

void initcar_init() {
	km_result_t result;

	kf_uuid_t uuid = INITCAR_UUID;
	if (!(initcar_fs = fs_register_filesys("initcar", strlen("initcar"), &uuid, &initcar_ops)))
		km_panic("Error registering initcar file system");

	kdprintf("INITCAR range: %p-%p\n",
		ARCH_KARGS_PTR->initcar_ptr,
		((const char *)ARCH_KARGS_PTR->initcar_ptr) + ARCH_KARGS_PTR->initcar_size);

	size_t sz_left = ARCH_KARGS_PTR->initcar_size;

	if (!(initcar_ptr = mm_vmalloc(
			  mm_kernel_context,
			  (const char *)CRITICAL_VTOP + 1,
			  (const void *)UINTPTR_MAX,
			  ARCH_KARGS_PTR->initcar_size,
			  PAGE_MAPPED | PAGE_READ,
			  0)))
		km_panic("Error allocating virtual memory space for INITCAR");

	mm_mmap(mm_kernel_context, initcar_ptr, ARCH_KARGS_PTR->initcar_ptr, ARCH_KARGS_PTR->initcar_size, PAGE_MAPPED | PAGE_READ, 0);

	pbcar_metadata_t *md = (pbcar_metadata_t*)initcar_ptr;
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
	const char *p_cur = ((const char *)initcar_ptr) + sizeof(pbcar_metadata_t);
	const uint32_t initcar_size = ARCH_KARGS_PTR->initcar_size;

#define initcar_checksize(size)                                      \
	if (((p_cur - (const char *)initcar_ptr) + size) > initcar_size) \
		km_panic("Prematured end of file\n");

	if (KM_FAILED(result = kn_alloc_file(initcar_fs, "initcar", sizeof("initcar") - 1, FS_FILETYPE_DIR, sizeof(initcar_dir_exdata_t), &initcar_dir)))
		km_panic("Error creating initcar directory, error code = %.0x", result);
	kf_hashmap_init(
		&((initcar_dir_exdata_t *)initcar_dir->exdata)->children,
		kn_initcar_file_hasher,
		kn_initcar_file_nodefree,
		kn_initcar_file_nodecmp,
		NULL);

	{
		// fs_file_t * root_handle;
		// if (KM_FAILED(fs_open("/", sizeof("/") - 1, &root_handle)))
		// km_panic("Error opening the root directory, error code = %.0x", result);
		if (KM_FAILED(result = fs_mount_file(fs_abs_root_dir, initcar_dir)))
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

		fs_file_t *file;
		size_t filename_len = strlen(fe->filename);
		if (KM_FAILED(kn_alloc_file(
				initcar_fs,
				fe->filename, filename_len,
				FS_FILETYPE_FILE,
				sizeof(initcar_file_exdata_t) + filename_len,
				&file)))
			km_panic("Error creating file object for initcar file: %s\n", fe->filename);

		initcar_file_exdata_t *exdata = (initcar_file_exdata_t *)fs_file_exdata(file);

		exdata->ptr = p_cur;
		exdata->sz_total = fe->size;

		kdprintf("initcar: Mounting file: %s\n", fe->filename);
		if (KM_FAILED(result = fs_mount_file(initcar_dir, file)))
			km_panic("Error mounting initcar file `%s', error code = %x\n", fe->filename, result);

		p_cur += fe->size;
	}
}

void initcar_deinit() {
	mm_unmmap(mm_kernel_context, initcar_ptr, ARCH_KARGS_PTR->initcar_size, 0);
}

PBOS_EXTERN_C_END
