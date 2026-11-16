#include "shell.h"

typedef struct {
    pid_t pid;
    char cmd[MAX_LEN];
} Job;

static Job jobs[MAX_JOBS];
static int job_count = 0;

void jobs_add(pid_t pid, const char* cmdline) {
    if (job_count >= MAX_JOBS) {
        fprintf(stderr, "jobs: too many background jobs\n");
        return;
    }
    jobs[job_count].pid = pid;
    snprintf(jobs[job_count].cmd, sizeof(jobs[job_count].cmd), "%s", cmdline);
    job_count++;
}

void jobs_reap(void) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < job_count; i++) {
            if (jobs[i].pid == pid) {
                for (int j = i; j < job_count - 1; j++) {
                    jobs[j] = jobs[j + 1];
                }
                job_count--;
                break;
            }
        }
    }
}

void jobs_print(void) {
    if (job_count == 0) {
        printf("No background jobs.\n");
        return;
    }
    for (int i = 0; i < job_count; i++) {
        printf("[%d] %d %s\n", i + 1, jobs[i].pid, jobs[i].cmd);
    }
}
