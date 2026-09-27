#include "executor.h"
#include "builtins.h"
#include "jobs.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <termios.h>

static int apply_redirs(Command*c){for(Redir*r=c->redirs;r;r=r->next){int flags=0;mode_t mode=0666;if(r->type==REDIR_IN)flags=O_RDONLY;else if(r->type==REDIR_OUT)flags=O_WRONLY|O_CREAT|O_TRUNC;else if(r->type==REDIR_APPEND)flags=O_WRONLY|O_CREAT|O_APPEND;else if(r->type==REDIR_ERR)flags=O_WRONLY|O_CREAT|O_TRUNC;else flags=O_WRONLY|O_CREAT|O_APPEND;int fd=open(r->path,flags,mode);if(fd<0){fprintf(stderr,"c2sh: %s: %s\n",r->path,strerror(errno));return -1;}if(dup2(fd,r->fd)<0){perror("c2sh: dup2");close(fd);return -1;}close(fd);}return 0;}
static int run_builtin_parent(Shell*sh,Command*c){int save[3]={-1,-1,-1};for(Redir*r=c->redirs;r;r=r->next){if(r->fd>=0&&r->fd<3&&save[r->fd]<0)save[r->fd]=dup(r->fd);}if(apply_redirs(c)<0){for(int i=0;i<3;i++)if(save[i]>=0){dup2(save[i],i);close(save[i]);}return 1;}int rc=builtin_run(sh,c);fflush(NULL);for(int i=0;i<3;i++)if(save[i]>=0){dup2(save[i],i);close(save[i]);}return rc;}
static int execute_pipeline(Shell*sh,Pipeline*p,const char*source){if(p->count==1&&builtin_lookup(p->commands[0]->argv[0])&& !p->background)return run_builtin_parent(sh,p->commands[0]);
 size_t n=p->count;int (*pipes)[2]=n>1?calloc(n-1,sizeof(int[2])):NULL;if(n>1&&!pipes)die_oom();for(size_t i=0;i+1<n;i++)if(pipe(pipes[i])<0){perror("c2sh: pipe");free(pipes);return 1;}
 pid_t pgid=0,last=0;for(size_t i=0;i<n;i++){pid_t pid=fork();if(pid<0){perror("c2sh: fork");if(pgid)kill(-pgid,SIGTERM);free(pipes);return 1;}if(pid==0){signal(SIGINT,SIG_DFL);signal(SIGQUIT,SIG_DFL);signal(SIGTSTP,SIG_DFL);signal(SIGTTIN,SIG_DFL);signal(SIGTTOU,SIG_DFL);signal(SIGCHLD,SIG_DFL);if(pgid==0)pgid=getpid();setpgid(0,pgid);if(sh->interactive&&!p->background&&i==0)tcsetpgrp(sh->terminal_fd,pgid);
   if(i>0) dup2(pipes[i-1][0],STDIN_FILENO);
   if(i+1<n) dup2(pipes[i][1],STDOUT_FILENO);
   for(size_t k=0;k+1<n;k++){ close(pipes[k][0]); close(pipes[k][1]); }
   if(apply_redirs(p->commands[i])<0) _exit(126);
   int b=builtin_lookup(p->commands[i]->argv[0]);if(b){int rc=builtin_run(sh,p->commands[i]);_exit(rc<0?127:rc);}execvp(p->commands[i]->argv[0],p->commands[i]->argv);fprintf(stderr,"c2sh: %s: %s\n",p->commands[i]->argv[0],strerror(errno));_exit(errno==ENOENT?127:126);
 }else{if(pgid==0)pgid=pid;setpgid(pid,pgid);last=pid;}}
 if(n>1) for(size_t i=0;i+1<n;i++){ close(pipes[i][0]); close(pipes[i][1]); }
 free(pipes);
 Job*j=job_add(sh,pgid,last,p->background,source?source:p->commands[0]->argv[0]);if(p->background){printf("[%d] %d\n",j->id,(int)pgid);return 0;}
 if(sh->interactive) tcsetpgrp(sh->terminal_fd,pgid);
 int status=0,laststatus=0,stopped=0;size_t remaining=n;while(remaining){pid_t r=waitpid(-pgid,&status,WUNTRACED);if(r<0){if(errno==EINTR)continue;if(errno==ECHILD)break;perror("c2sh: waitpid");break;}if(WIFSTOPPED(status)){stopped=1;j->state=JOB_STOPPED;break;}if(r==last)laststatus=WIFEXITED(status)?WEXITSTATUS(status):128+WTERMSIG(status);remaining--;}
 if(sh->interactive) tcsetpgrp(sh->terminal_fd,sh->shell_pgid);
 if(stopped){j->background=1;fprintf(stderr,"\n[%d]+ Stopped %s\n",j->id,j->command);return 128+SIGTSTP;}job_remove(sh,j);return laststatus;}
int execute_command_list(Shell*sh,CommandList*list,const char*source){int rc=sh->last_status;for(size_t i=0;i<list->count;i++){Condition cond=list->conditions[i];if((cond==COND_AND&&rc!=0)||(cond==COND_OR&&rc==0))continue;expand_pipeline(list->items[i],sh->last_status);rc=execute_pipeline(sh,list->items[i],source);sh->last_status=rc;shell_reap_jobs(sh,1);if(sh->should_exit)break;}return rc;}
