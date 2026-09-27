#ifndef C2SH_BUILTINS_H
#define C2SH_BUILTINS_H
#include "shell.h"
#include "parser.h"
int builtin_lookup(const char *name);
int builtin_run(Shell *sh, Command *cmd);
#endif
