#ifndef C2SH_EXECUTOR_H
#define C2SH_EXECUTOR_H
#include "shell.h"
#include "parser.h"
int execute_command_list(Shell *sh, CommandList *list, const char *source);
#endif
