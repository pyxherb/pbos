#include <pbos/fmt/pbcar.h>
#include <stdio.h>
#include <string.h>
#include "op.h"

int op_create(int argc, char **argv) {
	if (argc < 4) {
		puts("Error: Missing arguments.");
		return -1;
	}
	const char *ar_filename = argv[2];
	const char *ar_flags = argv[3];

	FILE *fp = fopen(ar_filename, "wb");

	if (!fp) {
		puts("Error: Unable to open output file.");
		return -1;
	}

	// Initialize metadata.
	pbcar_metadata_t metadata;

	memset(&metadata, 0, sizeof(metadata));

	metadata.magic[0] = PBCAR_MAGIC_0;
	metadata.magic[1] = PBCAR_MAGIC_1;
	metadata.magic[2] = PBCAR_MAGIC_2;
	metadata.magic[3] = PBCAR_MAGIC_3;

	metadata.major_ver = 0;
	metadata.minor_ver = 1;

	// Scan for each flag.
	for (int i = 0; i < strlen(ar_flags); ++i)
		switch (ar_flags[i]) {
			case '_':
				break;
			default:
				fclose(fp);
				printf("Error: Invalid archive flag '%c' detected!\n", ar_flags[i]);
				return -1;
		}

	// Save the metadata.
	if (!fwrite(&metadata, sizeof(metadata), 1, fp)) {
		fclose(fp);
		puts("Error: Error writing file.");
		return -1;
	}

	// Scan files.
	for (int i = 4; i < argc; ++i) {
		const char *src_filename = strrchr(argv[i], '/');

		// Exclude directory part in the path.
		if (!src_filename)
			src_filename = argv[i];
		else
			src_filename++;

		// Open the source file.
		FILE *src_fp = fopen(argv[i], "rb");

		if (!src_fp) {
			fclose(fp);
			printf("Error: Error opening source file '%s'.\n", src_filename);
			return -1;
		}

		// Get file size.
		fseek(src_fp, 0, SEEK_END);
		long sz_file = ftell(src_fp);
		rewind(src_fp);

		// Initialize file entry.
		pbcar_fentry_t file_entry;

		memset(&file_entry, 0, sizeof(file_entry));

		strncpy(file_entry.filename, src_filename, 31);
		file_entry.filename[sizeof(file_entry.filename) - 1] = '\0';
		file_entry.size = sz_file;

		if (!fwrite(&file_entry, sizeof(file_entry), 1, fp)) {
			fclose(src_fp);
			fclose(fp);
			puts("Error: Error writing file.");
			return -1;
		}

		// Write source file contents into the archive.

		// Show file's overview.
		printf("+ %s\n", src_filename);

		// Copy the contents as blocks.
		char buffer[8192];

		while (sz_file > 0)
			if (sz_file < sizeof(buffer)) {
				if (!fread(buffer, sz_file, 1, src_fp)) {
					fclose(src_fp);
					fclose(fp);
					printf("Error: Error reading file '%s'.\n", src_filename);
					return -1;
				}

				if (!fwrite(buffer, sz_file, 1, fp)) {
					fclose(src_fp);
					fclose(fp);
					puts("Error: Error writing file.");
					return -1;
				}

				sz_file = 0;
			} else {
				if (!fread(buffer, sizeof(buffer), 1, src_fp)) {
					fclose(src_fp);
					fclose(fp);
					printf("Error: Error reading file '%s'.\n", src_filename);
					return -1;
				}

				if (!fwrite(buffer, sizeof(buffer), 1, fp)) {
					fclose(src_fp);
					fclose(fp);
					puts("Error: Error writing file.");
					return -1;
				}

				sz_file -= sizeof(buffer);
			}

		fclose(src_fp);
	}
	pbcar_fentry_t end_file_entry;
	memset(&end_file_entry, 0, sizeof(end_file_entry));

	strcpy(end_file_entry.filename, "");
	end_file_entry.flags |= PBCAR_FILE_FLAG_END;

	if (!fwrite(&end_file_entry, sizeof(end_file_entry), 1, fp)) {
		fclose(fp);
		puts("Error: Error writing file.");
		return -1;
	}
	fclose(fp);

	return 0;
}
