#ifndef OP_H
#define OP_H

typedef struct _app_cmd_t {
    const char* name;
    int (*proc)(int argc, char** argv);
    const char* help;
} app_cmd_t;

extern app_cmd_t app_cmds[];

int op_create(int argc, char **argv);
int op_help(int argc, char **argv);
int op_extract(int argc, char **argv);

#endif
