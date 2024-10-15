#include "op.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdbool.h>
#include <pbos/fmt/pbkim.h>

int op_create(int argc, char **argv)
{
	if (argc != 5)
	{
		puts("Error: Incorrect argument count");
		return 1;
	}

	char *boot_bin_filename = argv[2];
	char *kimage_filename = argv[3];
	char *out_filename = argv[4];

	//
	// Read the bootstrap code.
	//
	FILE *fp_bootcode = NULL, *fp_kimage = NULL, *fp_pbkim = NULL;
	size_t sz_bootcode, sz_kimage;
	void *buf_bootcode = NULL, *buf_kimage = NULL;

	if (!(fp_bootcode = fopen(boot_bin_filename, "rb")))
	{
		puts("Error: Failed to open the bootstrap code file");
		goto fail;
	}

	// Get file size of the bootstrap code.
	fseek(fp_bootcode, 0, SEEK_END);
	sz_bootcode = ftell(fp_bootcode);
	rewind(fp_bootcode);

	// Check if the bootstrap code size is not bigger than 64 KiB (1024*64 Bytes=65536 Bytes)
	if (sz_bootcode > (1024 * 64))
	{
		puts("Error: bootstrap code is too big");
		goto fail;
	}

	// The bootstrap code buffer allocates here.
	buf_bootcode = calloc(1024, 64);
	if (!buf_bootcode)
	{
		puts("Error: Out of memory");
		goto fail;
	}

	memset(buf_bootcode, 0, 1024 * 64);

	// Read bootstrap code file.
	if (!fread(buf_bootcode, sz_bootcode, 1, fp_bootcode))
	{
		puts("Error: Error reading bootstrap code file");
		goto fail;
	}

	fclose(fp_bootcode);

	//
	// Read the kernel image.
	//
	if (!(fp_kimage = fopen(kimage_filename, "rb")))
	{
		puts("Error: Failed to open the kernel image file");
		goto fail;
	}

	// Get file size of the kernel image.
	fseek(fp_kimage, 0, SEEK_END);
	sz_kimage = ftell(fp_kimage);
	rewind(fp_kimage);

	// The kernel image buffer allocates here.
	buf_kimage = malloc(sz_kimage);
	if (!buf_kimage)
	{
		puts("Error: Out of memory");
		goto fail;
	}

	memset(buf_kimage, 0, sz_kimage);

	// Read kernel image file.
	if (!fread(buf_kimage, sz_kimage, 1, fp_kimage))
	{
		puts("Error: Error reading kernel image file");
		goto fail;
	}

	fclose(fp_kimage);
	fp_kimage = NULL;

	//
	// Combine the parts together.
	//
	if (!(fp_pbkim = fopen(out_filename, "wb")))
	{
		free(buf_kimage);
		free(buf_bootcode);
		puts("Error: Failed to open the OICKIM file.");
		return 1;
	}

	// Set the image header.
	pbkim_ihdr_t ih;
	memset(&ih, 0, sizeof(ih));

	ih.magic[0] = OICKIM_MAGIC_0;
	ih.magic[1] = OICKIM_MAGIC_1;
	ih.magic[2] = OICKIM_MAGIC_2;
	ih.magic[3] = OICKIM_MAGIC_3;

	ih.endian = false;

	ih.image_size = sz_kimage;
	ih.machine = OICKIM_MACHINE_I386;

	// Write all data.
	if (!fwrite(buf_bootcode, 1024 * 64, 1, fp_pbkim))
	{
		puts("Error: Error writing file");
		goto fail;
	}

	if (!fwrite(&ih, sizeof(ih), 1, fp_pbkim))
	{
		puts("Error: Error writing file");
		goto fail;
	}

	if (!fwrite(buf_kimage, sz_kimage, 1, fp_pbkim))
	{
		puts("Error: Error writing file");
		goto fail;
	}

	fclose(fp_pbkim);
	free(buf_kimage);
	free(buf_bootcode);

	return 0;
fail:
	if(fp_bootcode)
		fclose(fp_bootcode);
	if(fp_pbkim)
		fclose(fp_pbkim);
	if(buf_kimage)
		free(buf_kimage);
	if(buf_bootcode)
		free(buf_bootcode);
	return -1;
}
