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

char** tokenize(const char* cmdline) {
    if (cmdline == NULL) {
        return NULL;
    }

    int cap = 16;
    int count = 0;
    char** tokens = malloc(sizeof(char*) * cap);
    const char* cp = cmdline;

    while (*cp != '\0') {
        while (*cp == ' ' || *cp == '\t') {
            cp++;
        }
        if (*cp == '\0') {
            break;
        }

        if (strchr(";|&<>", *cp) != NULL) {
            char special[2] = {*cp, '\0'};
            if (count + 1 >= cap) {
                cap *= 2;
                tokens = realloc(tokens, sizeof(char*) * cap);
            }
            tokens[count++] = strdup(special);
            cp++;
            continue;
        }

        char* buf = malloc(strlen(cmdline) + 1);
        int bi = 0;
        int quote = 0;
        int had_char = 0;

        while (*cp != '\0') {
            char c = *cp;
            if (quote != 0) {
                if (c == quote) {
                    quote = 0;
                    cp++;
                    had_char = 1;
                } else {
                    buf[bi++] = c;
                    cp++;
                    had_char = 1;
                }
            } else if (c == '\'' || c == '"') {
                quote = c;
                cp++;
                had_char = 1;
            } else if (c == ' ' || c == '\t' || strchr(";|&<>", c) != NULL) {
                break;
            } else {
                buf[bi++] = c;
                cp++;
                had_char = 1;
            }
        }

        if (had_char) {
            buf[bi] = '\0';
            if (count + 1 >= cap) {
                cap *= 2;
                tokens = realloc(tokens, sizeof(char*) * cap);
            }
            tokens[count++] = strdup(buf);
        }
        free(buf);
    }

    if (count == 0) {
        free(tokens);
        return NULL;
    }

    tokens[count] = NULL;
    return tokens;
}

void free_tokens(char** tokens) {
    if (tokens == NULL) {
        return;
    }
    for (int i = 0; tokens[i] != NULL; i++) {
        free(tokens[i]);
    }
    free(tokens);
}
