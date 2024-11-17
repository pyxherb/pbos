#include "op.h"

#include <pbos/fmt/pbcar.h>
#include <stdio.h>
#include <string.h>

int op_extract(int argc, char** argv) {
	if (argc < 3 || argc > 4) {
		puts("Error: Incorrect argument count");
		return -1;
	}

	const char* src_filename = argv[2];
	const char* dest = ".";

	if (argc > 3)
		dest = argv[3];

	FILE* src_fp = fopen(src_filename, "rb");

	pbcar_metadata_t md;

	if (!fread(&md, sizeof(md), 1, src_fp)) {
		fclose(src_fp);
		puts("Error: Error reading the archive");
		return 1;
	}

	if (md.magic[0] != PBCAR_MAGIC_0 || md.magic[1] != PBCAR_MAGIC_1 || md.magic[2] != PBCAR_MAGIC_2 ||
		md.magic[3] != PBCAR_MAGIC_3) {
		fclose(src_fp);
		puts("Error: Error reading the archive");
		return 1;
	}

	while (true) {
		pbcar_fentry_t fe;
		if (!fread(&fe, sizeof(fe), 1, src_fp)) {
			fclose(src_fp);
			puts("Error: Error reading the archive");
			return 1;
		}
		if (!strlen(fe.filename))
			break;

		fe.filename[sizeof(fe.filename) - 1] = '\0';

		printf("> %s\n", fe.filename);

		FILE* fp = fopen(fe.filename, "wb");
		if (!fp) {
			fclose(src_fp);
			puts("Error: Error writing file");
			return 1;
		}

		char buffer[8192];

		while (fe.size > 0)
			if (fe.size < sizeof(buffer)) {
				if (!fread(buffer, fe.size, 1, src_fp)) {
					fclose(src_fp);
					fclose(fp);
					puts("Error: Error reading archive");
					return -1;
				}

				if (!fwrite(buffer, fe.size, 1, fp)) {
					fclose(src_fp);
					fclose(fp);
					puts("Error: Error writing file.");
					return -1;
				}

				fe.size = 0;
			} else {
				if (!fread(buffer, sizeof(buffer), 1, src_fp)) {
					fclose(src_fp);
					fclose(fp);
					puts("Error: Error reading archive");
					return -1;
				}

				if (!fwrite(buffer, sizeof(buffer), 1, fp)) {
					fclose(src_fp);
					fclose(fp);
					puts("Error: Error writing file.");
					return -1;
				}

				fe.size -= sizeof(buffer);
			}

		fclose(fp);
	}

	fclose(src_fp);

	return 0;
}
