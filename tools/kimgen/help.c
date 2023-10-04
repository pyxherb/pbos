#include <stdio.h>

#include "op.h"

const char* msg_help = "Usage: kimgen <operation> [parameters] ...\n"
						   "\n"
						   "Operations:\n"
						   "create <Bootstrap Binary> <Kernel Image> <Output>\n"
						   "\tCreate a new image.\n"
						   "extract <File> [Destination]\n"
						   "\tExtract an image.\n"
						   "help\n"
						   "\tShow this help.\n";

int op_help(int argc, char** argv) {
	puts(msg_help);
	return 0;
}
