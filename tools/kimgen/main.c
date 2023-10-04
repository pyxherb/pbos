#include <stdio.h>
#include <string.h>

#include "op.h"

app_cmd_t app_cmds[] = {
	{ .name = "create", .proc = op_create, .help = "Create a kernel image" },
	{ .name = "extract", .proc = op_extract, .help = "Extract files from a kernel image" },
	{ .name = "help", .proc = op_help, .help = "Show this help" },
	{ NULL }
};

int main(int argc, char** argv) {
	if (argc < 2) {
		puts("Error: Too few arguments.");
		puts("Type 'kimgen help' to get help.");
		return 1;
	}

	const char* cmd = argv[1];

	for (app_cmd_t* i = app_cmds; i->name != NULL; ++i) {
		if (!strcmp(cmd, i->name))
			return i->proc(argc, argv);
	}

	puts("Error: Unknown command.");
	puts("Enter 'kimgen help' to get help.");
	return -1;
}
