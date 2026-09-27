#include "shell.h"
#include <stdio.h>
int main(int argc,char**argv,char**envp){Shell sh;shell_init(&sh,argc,argv,envp);int rc=shell_run(&sh,argc,argv);shell_destroy(&sh);return rc;}
