#include "shell.h"
#include "lexer.h"
#include "parser.h"
#include "editor.h"
#include "executor.h"
#include "jobs.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#include <sys/wait.h>
extern void history_init(Shell*);extern void history_add(Shell*,const char*);extern void history_free(Shell*);extern char*make_prompt(Shell*);
static Shell*global_shell;
static void on_sigchld(int sig){(void)sig;}
static void init_job_control(Shell*sh){sh->terminal_fd=STDIN_FILENO;sh->shell_pgid=getpid();if(!sh->interactive)return;while(tcgetpgrp(sh->terminal_fd)!=(sh->shell_pgid=getpgrp()))kill(-sh->shell_pgid,SIGTTIN);setpgid(sh->shell_pgid,sh->shell_pgid);tcsetpgrp(sh->terminal_fd,sh->shell_pgid);signal(SIGINT,SIG_IGN);signal(SIGQUIT,SIG_IGN);signal(SIGTSTP,SIG_IGN);signal(SIGTTIN,SIG_IGN);signal(SIGTTOU,SIG_IGN);signal(SIGCHLD,on_sigchld);}
void shell_init(Shell*sh,int argc,char**argv,char**envp){(void)argv;memset(sh,0,sizeof(*sh));sh->envp=envp;/* -c and script modes must never take terminal control. */sh->interactive=argc<2&&isatty(STDIN_FILENO)&&isatty(STDOUT_FILENO);sh->last_status=0;jobs_init(sh);history_init(sh);init_job_control(sh);global_shell=sh;}
void shell_destroy(Shell*sh){history_free(sh);jobs_destroy(sh);global_shell=NULL;}
void shell_reap_jobs(Shell*sh,int notify){int status;pid_t p;while((p=waitpid(-1,&status,WNOHANG|WUNTRACED|WCONTINUED))>0){Job*j=NULL;for(Job*x=sh->jobs;x;x=x->next){if(x->pgid==getpgid(p)||x->last_pid==p){j=x;break;}}if(!j)continue;if(WIFSTOPPED(status))j->state=JOB_STOPPED;else if(WIFCONTINUED(status))j->state=JOB_RUNNING;else if(p==j->last_pid){j->state=JOB_DONE;if(notify){printf("[%d] Done %s\n",j->id,j->command);fflush(stdout);}job_remove(sh,j);}}}
int shell_execute_line(Shell*sh,const char*line){char*trim=trim_copy(line);if(!*trim){free(trim);return 0;}TokenList tl;char*err=NULL;if(lex_line(trim,&tl,&err)<0){fprintf(stderr,"c2sh: syntax: %s\n",err);free(err);free(trim);return 2;}CommandList cl;if(parse_tokens(&tl,&cl,&err)<0){fprintf(stderr,"c2sh: syntax: %s\n",err);free(err);token_list_free(&tl);free(trim);return 2;}history_add(sh,trim);int rc=execute_command_list(sh,&cl,trim);command_list_free(&cl);token_list_free(&tl);free(trim);return rc;}
int shell_run(Shell*sh,int argc,char**argv){if(argc>=3&&!strcmp(argv[1],"-c")){int rc=shell_execute_line(sh,argv[2]);return sh->should_exit?sh->exit_status:rc;}if(argc>=2){FILE*f=fopen(argv[1],"r");if(!f){perror(argv[1]);return 1;}char*l=NULL;size_t n=0;int rc=0;while(getline(&l,&n,f)>0){rc=shell_execute_line(sh,l);if(sh->should_exit)break;}free(l);fclose(f);return sh->should_exit?sh->exit_status:rc;}
 if(!sh->interactive){char*l=NULL;size_t n=0;int rc=0;while(getline(&l,&n,stdin)>0){rc=shell_execute_line(sh,l);if(sh->should_exit)break;}free(l);return sh->should_exit?sh->exit_status:rc;}
 const char*no_rc=getenv("C2SH_NORC"); if(!no_rc){const char*home=getenv("HOME");if(home){char path[4096];snprintf(path,sizeof(path),"%s/.c2shrc",home);FILE*rcf=fopen(path,"r");if(rcf){char*l=NULL;size_t n=0;while(getline(&l,&n,rcf)>0){shell_execute_line(sh,l);if(sh->should_exit)break;}free(l);fclose(rcf);}}}
 while(!sh->should_exit){shell_reap_jobs(sh,1);char*p=make_prompt(sh);char*line=editor_readline(sh,p);free(p);if(!line)break;if(!*line){free(line);continue;}sh->last_status=shell_execute_line(sh,line);free(line);}return sh->should_exit?sh->exit_status:sh->last_status;}
