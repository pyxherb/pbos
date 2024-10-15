/**
 * @file kblrgen.c
 * @author Pyxherb (codesbuilder@163.com)
 * @brief Source of main module of Kernel Binary Loader Registry Generator (KBLRGEN)
 * @version 0.1
 * @date 2023-08-11
 *
 * @copyright Copyright (c) 2023 PbOS Contributors
 *
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		fputs("Incorrect usage\n", stderr);
		return -1;
	}

	puts("#include <insipid/kn/km/exec.h>\n");

	puts("extern km_binldr_t ");
	for (int i = 1; i < argc; ++i) {
		const char* name = argv[i];

		printf("\tkn_binldr_%s", name);
		if(i + 1 < argc)
			puts(", ");
	}

	puts(";");

	puts("\n"
		"km_binldr_t* kn_builtin_binldrs[] = {");

	for (int i = 1; i < argc; ++i)
	{
		const char *name = argv[i];

		printf("\t&kn_binldr_%s,\n", name);
	}

	puts(
		"\tNULL,\n"
		"};");

	return 0;
}
