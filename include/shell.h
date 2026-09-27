#ifndef C2SH_SHELL_H
#define C2SH_SHELL_H
#include <stddef.h>
#include <sys/types.h>

typedef struct Shell Shell;
typedef struct Job Job;

typedef enum { JOB_RUNNING, JOB_STOPPED, JOB_DONE } JobState;

typedef struct {
    char *text;
    int exit_status;
} HistoryEntry;

struct Job {
    int id;
    pid_t pgid;
    pid_t last_pid;
    JobState state;
    int background;
    char *command;
    Job *next;
};

struct Shell {
    int interactive;
    int last_status;
    int should_exit;
    int exit_status;
    int terminal_fd;
    pid_t shell_pgid;
    char **envp;
    Job *jobs;
    int next_job_id;
    HistoryEntry *history;
    size_t history_len;
    size_t history_cap;
    char *history_path;
    int history_limit;
};

void shell_init(Shell *sh, int argc, char **argv, char **envp);
void shell_destroy(Shell *sh);
int shell_run(Shell *sh, int argc, char **argv);
int shell_execute_line(Shell *sh, const char *line);
void shell_reap_jobs(Shell *sh, int notify);

#endif
