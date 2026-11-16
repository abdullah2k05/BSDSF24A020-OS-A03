#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>

static void load_command_names(void);

char* read_cmd(const char* prompt) {
    load_command_names();
    char* cmdline = readline(prompt);
    if (cmdline == NULL) {
        return NULL; // Ctrl+D
    }
    if (cmdline[0] != '\0') {
        add_history(cmdline);
    }
    return cmdline;
}

static char** shell_completion(const char* text, int start, int end);
static char* command_generator(const char* text, int state);

static char** command_names = NULL;
static int command_count = 0;

static void add_command_name(const char* name) {
    for (int i = 0; i < command_count; i++) {
        if (strcmp(command_names[i], name) == 0) {
            return;
        }
    }
    command_names = realloc(command_names, sizeof(char*) * (command_count + 1));
    command_names[command_count++] = strdup(name);
}

static void load_command_names(void) {
    if (command_names != NULL) {
        return;
    }

    const char* builtins[] = {"cd", "exit", "help", "history", "jobs", NULL};
    for (int i = 0; builtins[i] != NULL; i++) {
        add_command_name(builtins[i]);
    }

    char* path = strdup(getenv("PATH") ? getenv("PATH") : "/usr/bin:/bin");
    char* dir = strtok(path, ":");
    while (dir != NULL) {
        DIR* dp = opendir(dir);
        if (dp != NULL) {
            struct dirent* entry;
            while ((entry = readdir(dp)) != NULL) {
                if (entry->d_name[0] == '.') {
                    continue;
                }
                char full[1024];
                snprintf(full, sizeof(full), "%s/%s", dir, entry->d_name);
                if (access(full, X_OK) == 0) {
                    add_command_name(entry->d_name);
                }
            }
            closedir(dp);
        }
        dir = strtok(NULL, ":");
    }
    free(path);

    rl_attempted_completion_function = shell_completion;
}

static char* command_generator(const char* text, int state) {
    static int index;
    static int len;

    if (state == 0) {
        index = 0;
        len = strlen(text);
    }

    while (index < command_count) {
        const char* name = command_names[index++];
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }
    return NULL;
}

char** shell_completion(const char* text, int start, int end) {
    (void)end;
    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    }
    return rl_completion_matches(text, rl_filename_completion_function);
}

char** tokenize(char* cmdline) {
    // Edge case: empty command line
    if (cmdline == NULL || cmdline[0] == '\0' || cmdline[0] == '\n') {
        return NULL;
    }

    char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*)malloc(sizeof(char) * ARGLEN);
        bzero(arglist[i], ARGLEN);
    }

    char* cp = cmdline;
    char* start;
    int len;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++; // Skip leading whitespace
        
        if (*cp == '\0') break; // Line was only whitespace

        start = cp;
        len = 1;
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) {
            len++;
        }
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }

    if (argnum == 0) { // No arguments were parsed
        for(int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}