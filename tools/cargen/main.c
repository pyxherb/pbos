#include <stdio.h>
#include <string.h>

#include "op.h"

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		puts("Error: Too few arguments.");
		puts("Enter 'cargen -h' to get help.");
		return 1;
	}

	const char *operation = argv[1];

	if (!strcmp(operation, "-h"))
		return op_help(argc, argv);
	else if (!strcmp(operation, "-c"))
		return op_create(argc, argv);
	else if (!strcmp(operation, "-x"))
		return op_extract(argc, argv);

	puts("Error: Unknown operation.");
	puts("Enter 'cargen -h' to get help.");
	return -1;
}
