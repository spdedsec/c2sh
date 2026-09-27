#include "builtins.h"
#include "jobs.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <limits.h>
extern void history_save(Shell*);extern void history_add(Shell*,const char*);extern void history_free(Shell*);
enum {B_NONE=0,B_CD,B_PWD,B_ECHO,B_EXPORT,B_UNSET,B_ENV,B_HISTORY,B_JOBS,B_FG,B_BG,B_KILL,B_TYPE,B_WHICH,B_HELP,B_SOURCE,B_TRUE,B_FALSE,B_EXIT};
int builtin_lookup(const char*n){const char*names[]={"","cd","pwd","echo","export","unset","env","history","jobs","fg","bg","kill","type","which","help","source","true","false","exit"};for(int i=1;i<=B_EXIT;i++)if(!strcmp(n,names[i]))return i;return 0;}
static int bi_cd(Shell*sh,Command*c){(void)sh;const char*p=c->argc>1?c->argv[1]:getenv("HOME");if(!p)p=".";if(chdir(p)<0){perror("c2sh: cd");return 1;}return 0;}
static int bi_export(Command*c){if(c->argc==1){extern char**environ;for(char**e=environ;*e;e++)puts(*e);return 0;}for(int i=1;i<c->argc;i++){char*eq=strchr(c->argv[i],'=');if(!eq){const char*v=getenv(c->argv[i]);if(v)printf("%s=%s\n",c->argv[i],v);else fprintf(stderr,"c2sh: export: %s not set\n",c->argv[i]);continue;}*eq='\0';if(setenv(c->argv[i],eq+1,1)<0){perror("c2sh: export");*eq='=';return 1;}*eq='=';}return 0;}
static int bi_history(Shell*sh,Command*c){size_t start=0;if(c->argc>1)start=sh->history_len>(size_t)atoi(c->argv[1])?sh->history_len-(size_t)atoi(c->argv[1]):0;for(size_t i=start;i<sh->history_len;i++)printf("%5zu  %s\n",i+1,sh->history[i].text);return 0;}
static Job*parse_job(Shell*sh,const char*s){if(!s)return sh->jobs;if(s[0]=='%')s++;char*e;long id=strtol(s,&e,10);if(*e=='\0')return job_find_id(sh,(int)id);for(Job*j=sh->jobs;j;j=j->next)if(strstr(j->command,s)==j->command)return j;return NULL;}
static int bi_jobs(Shell*sh){shell_reap_jobs(sh,0);for(Job*j=sh->jobs;j;j=j->next)job_print(j);return 0;}
static int bi_fg(Shell*sh,Command*c){Job*j=parse_job(sh,c->argc>1?c->argv[1]:NULL);if(!j){fprintf(stderr,"c2sh: fg: job not found\n");return 1;}return job_run_fg(sh,j);}
static int bi_bg(Shell*sh,Command*c){Job*j=parse_job(sh,c->argc>1?c->argv[1]:NULL);if(!j){fprintf(stderr,"c2sh: bg: job not found\n");return 1;}return job_run_bg(sh,j);}
static int bi_kill(Shell*sh,Command*c){if(c->argc<2){fprintf(stderr,"usage: kill [-SIGNAL] %%job|PID\n");return 2;}int sig=SIGTERM;int idx=1;if(c->argv[1][0]=='-'){sig=atoi(c->argv[1]+1);idx++;}for(;idx<c->argc;idx++){if(c->argv[idx][0]=='%'){Job*j=parse_job(sh,c->argv[idx]);if(!j||kill(-j->pgid,sig)<0){perror("c2sh: kill");return 1;}}else{pid_t p=(pid_t)strtol(c->argv[idx],NULL,10);if(kill(p,sig)<0){perror("c2sh: kill");return 1;}}}return 0;}
static int bi_type(Command*c){for(int i=1;i<c->argc;i++){int b=builtin_lookup(c->argv[i]);if(b){printf("%s is a c2sh builtin\n",c->argv[i]);continue;}char*path=getenv("PATH");char*dup=path?strdup(path):NULL;int found=0;for(char*t=dup?strtok(dup,":"):NULL;t;t=strtok(NULL,":")){char p[PATH_MAX];snprintf(p,sizeof(p),"%s/%s",t,c->argv[i]);if(access(p,X_OK)==0){printf("%s is %s\n",c->argv[i],p);found=1;break;}}free(dup);if(!found)printf("%s not found\n",c->argv[i]);}return 0;}
static int bi_source(Shell*sh,Command*c){if(c->argc<2){fprintf(stderr,"usage: source FILE\n");return 2;}FILE*f=fopen(c->argv[1],"r");if(!f){perror("c2sh: source");return 1;}char*l=NULL;size_t n=0;int rc=0;while(getline(&l,&n,f)>0){rc=shell_execute_line(sh,l);if(sh->should_exit)break;}free(l);fclose(f);return rc;}
static int bi_help(void){puts("c2sh builtins: cd pwd echo export unset env history jobs fg bg kill type which help source alias unalias true false exit");puts("Operators: | || && ; & < > >> 2> 2>>");puts("Editing: arrows, Home/End, Backspace/Delete, Tab completion, Ctrl-R history search, Ctrl-C cancel");return 0;}
int builtin_run(Shell*sh,Command*c){int b=builtin_lookup(c->argv[0]);switch(b){case B_CD:return bi_cd(sh,c);case B_PWD:{char p[PATH_MAX];if(getcwd(p,sizeof(p))){puts(p);return 0;}perror("c2sh: pwd");return 1;}case B_ECHO:for(int i=1;i<c->argc;i++)printf("%s%s",i>1?" ":"",c->argv[i]);putchar('\n');return 0;case B_EXPORT:return bi_export(c);case B_UNSET:for(int i=1;i<c->argc;i++)unsetenv(c->argv[i]);return 0;case B_ENV:if(c->argc>1){execvp(c->argv[1],c->argv+1);perror("c2sh: env");return 127;}return bi_export(c);case B_HISTORY:return bi_history(sh,c);case B_JOBS:return bi_jobs(sh);case B_FG:return bi_fg(sh,c);case B_BG:return bi_bg(sh,c);case B_KILL:return bi_kill(sh,c);case B_TYPE:case B_WHICH:return bi_type(c);case B_HELP:return bi_help();case B_SOURCE:return bi_source(sh,c);case B_TRUE:return 0;case B_FALSE:return 1;case B_EXIT:sh->should_exit=1;sh->exit_status=c->argc>1?atoi(c->argv[1]):sh->last_status;return sh->exit_status;default:return -1;}}
