#include "jobs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include "util.h"
static const char*state(JobState s){return s==JOB_RUNNING?"Running":s==JOB_STOPPED?"Stopped":"Done";}
void jobs_init(Shell*sh){sh->jobs=NULL;sh->next_job_id=1;}
Job*job_add(Shell*sh,pid_t pgid,pid_t last_pid,int background,const char*command){Job*j=calloc(1,sizeof(*j));if(!j)die_oom();j->id=sh->next_job_id++;j->pgid=pgid;j->last_pid=last_pid;j->state=JOB_RUNNING;j->background=background;j->command=xstrdup(command);j->next=sh->jobs;sh->jobs=j;return j;}
Job*job_find_id(Shell*sh,int id){for(Job*j=sh->jobs;j;j=j->next)if(j->id==id)return j;return NULL;} Job*job_find_pgid(Shell*sh,pid_t p){for(Job*j=sh->jobs;j;j=j->next)if(j->pgid==p)return j;return NULL;}
void job_remove(Shell*sh,Job*j){Job**pp=&sh->jobs;while(*pp&&*pp!=j)pp=&(*pp)->next;if(*pp){*pp=j->next;free(j->command);free(j);}}
void job_print(const Job*j){printf("[%d] %-8s %s\n",j->id,state(j->state),j->command);}
int job_run_bg(Shell*sh,Job*j){(void)sh;if(!j)return 1;if(kill(-j->pgid,SIGCONT)<0&&errno!=ESRCH){perror("c2sh: SIGCONT");return 1;}j->state=JOB_RUNNING;j->background=1;printf("[%d] %d\n",j->id,(int)j->pgid);return 0;}
int job_run_fg(Shell*sh,Job*j){if(!j)return 1;j->background=0;j->state=JOB_RUNNING;if(sh->interactive)tcsetpgrp(sh->terminal_fd,j->pgid);if(kill(-j->pgid,SIGCONT)<0&&errno!=ESRCH)perror("c2sh: SIGCONT");int status=0;pid_t r;do{r=waitpid(-j->pgid,&status,WUNTRACED);}while(r<0&&errno==EINTR);if(WIFSTOPPED(status)){j->state=JOB_STOPPED;j->background=1;fprintf(stderr,"\n[%d]+ Stopped %s\n",j->id,j->command);}else{j->state=JOB_DONE;}if(sh->interactive)tcsetpgrp(sh->terminal_fd,sh->shell_pgid);if(j->state==JOB_DONE){int rc=WIFEXITED(status)?WEXITSTATUS(status):128+WTERMSIG(status);job_remove(sh,j);return rc;}return 128+SIGTSTP;}
void jobs_destroy(Shell*sh){Job*j=sh->jobs;while(j){Job*n=j->next;free(j->command);free(j);j=n;}sh->jobs=NULL;}
