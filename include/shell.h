#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "FCIT> "
#define HISTORY_SIZE 50
#define MAXPIPE 16
#define MAX_JOBS 64

typedef struct {
    char* args[MAXARGS + 1];
    int argc;
    char* infile;
    char* outfile;
} Command;

// Function prototypes
char* read_cmd(const char* prompt);
char** tokenize(const char* cmdline);
void free_tokens(char** tokens);
int execute_line(const char* line);
int execute_pipeline(Command* cmds, int ncmds, int background, const char* label);

// Background jobs
void jobs_add(pid_t pid, const char* cmdline);
void jobs_reap(void);
void jobs_print(void);
int handle_builtin(char** arglist);

// Command history
void history_add(const char* line);
char* shell_history_expand(const char* line);
void history_print(void);

#endif // SHELL_H
