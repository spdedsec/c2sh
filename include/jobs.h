#ifndef C2SH_JOBS_H
#define C2SH_JOBS_H
#include "shell.h"
void jobs_init(Shell *sh);
Job *job_add(Shell *sh, pid_t pgid, pid_t last_pid, int background, const char *command);
Job *job_find_id(Shell *sh, int id);
Job *job_find_pgid(Shell *sh, pid_t pgid);
void job_remove(Shell *sh, Job *job);
void job_print(const Job *job);
int job_run_fg(Shell *sh, Job *job);
int job_run_bg(Shell *sh, Job *job);
void jobs_destroy(Shell *sh);
#endif
