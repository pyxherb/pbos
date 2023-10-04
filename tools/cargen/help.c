#include "op.h"
#include <stdio.h>
#include <stdlib.h>

const char* msg_help="\
Usage: cargen <operation> [parameters] ...\n\
\n\
Operations:\n\
-c <File> <Flags> [Input File] ...\n\
	Create a new archive with input files.\n\
	Archive Flags:\n\
		_ - (None)\n\
-x <File> [Destination]\n\
	Extract an archive.\n\
-h\n\
	Show this help.\n\
\
";

int op_help(int argc, char **argv)
{
	puts(msg_help);
	return 0;
}
