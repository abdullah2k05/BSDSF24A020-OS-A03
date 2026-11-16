#include "shell.h"
#include <fcntl.h>
#include <ctype.h>

static int parse_pipeline(char** tokens, int* pos, Command* cmds, int* background) {
    int ncmds = 0;
    Command* cur = &cmds[0];
    memset(cur, 0, sizeof(Command));
    *background = 0;

    int i = *pos;
    for (; tokens[i] != NULL; i++) {
        char* tok = tokens[i];

        if (strcmp(tok, ";") == 0 || strcmp(tok, "&") == 0) {
            if (strcmp(tok, "&") == 0) {
                *background = 1;
            }
            i++;
            break;
        }

        if (strcmp(tok, "|") == 0) {
            ncmds++;
            if (ncmds >= MAXPIPE) {
                fprintf(stderr, "Too many pipe stages\n");
                return -1;
            }
            cur = &cmds[ncmds];
            memset(cur, 0, sizeof(Command));
        } else if (strcmp(tok, "<") == 0) {
            cur->infile = tokens[++i];
            if (cur->infile == NULL) {
                fprintf(stderr, "Syntax error: missing input file\n");
                return -1;
            }
        } else if (strcmp(tok, ">") == 0) {
            cur->outfile = tokens[++i];
            if (cur->outfile == NULL) {
                fprintf(stderr, "Syntax error: missing output file\n");
                return -1;
            }
        } else {
            if (cur->argc >= MAXARGS) {
                fprintf(stderr, "Too many arguments\n");
                return -1;
            }
            cur->args[cur->argc++] = tok;
        }
    }

    *pos = i;
    return ncmds + 1;
}

static void apply_redirection(Command* cmd) {
    if (cmd->infile != NULL) {
        int fd = open(cmd->infile, O_RDONLY);
        if (fd < 0) {
            perror(cmd->infile);
            _exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    if (cmd->outfile != NULL) {
        int fd = open(cmd->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            perror(cmd->outfile);
            _exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}

int execute_pipeline(Command* cmds, int ncmds, int background, const char* label) {
    int prev_read = -1;
    pid_t pids[MAXPIPE];

    for (int i = 0; i < ncmds; i++) {
        int pipefd[2] = {-1, -1};
        if (i < ncmds - 1) {
            if (pipe(pipefd) < 0) {
                perror("pipe");
                return 1;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }

        if (pid == 0) {
            if (prev_read != -1) {
                dup2(prev_read, STDIN_FILENO);
                close(prev_read);
            }
            if (i < ncmds - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
            }

            apply_redirection(&cmds[i]);

            execvp(cmds[i].args[0], cmds[i].args);
            perror(cmds[i].args[0]);
            _exit(127);
        }

        pids[i] = pid;

        if (prev_read != -1) {
            close(prev_read);
        }
        if (i < ncmds - 1) {
            close(pipefd[1]);
            prev_read = pipefd[0];
        } else {
            prev_read = -1;
        }
    }

    if (background) {
        jobs_add(pids[0], label);
        return 0;
    }

    int status = 0;
    for (int i = 0; i < ncmds; i++) {
        int child_status;
        waitpid(pids[i], &child_status, 0);
        if (i == ncmds - 1) {
            status = WIFEXITED(child_status) ? WEXITSTATUS(child_status) : 1;
        }
    }
    return status;
}

static char* join_tokens(char** tokens, int start, int end) {
    int len = 0;
    for (int i = start; i < end; i++) {
        len += strlen(tokens[i]) + 1;
    }
    char* out = malloc(len + 1);
    out[0] = '\0';
    for (int i = start; i < end; i++) {
        if (i > start) {
            strcat(out, " ");
        }
        strcat(out, tokens[i]);
    }
    return out;
}

static const char* trim_ws(const char* s) {
    while (*s == ' ' || *s == '\t') {
        s++;
    }
    return s;
}

static int starts_with_word(const char* s, const char* word) {
    s = trim_ws(s);
    int len = strlen(word);
    return strncmp(s, word, len) == 0 &&
           (s[len] == '\0' || isspace((unsigned char)s[len]));
}

static int execute_if_block(const char* line) {
    char* copy = strdup(line);
    char* lines[128];
    int nlines = 0;

    char* save = NULL;
    char* ln = strtok_r(copy, "\n", &save);
    while (ln != NULL && nlines < 128) {
        lines[nlines++] = ln;
        ln = strtok_r(NULL, "\n", &save);
    }

    if (nlines == 0) {
        free(copy);
        return 1;
    }

    char* condition = strdup(trim_ws(lines[0]) + 2);
    char* then_lines[64];
    char* else_lines[64];
    int nthen = 0;
    int nelse = 0;
    int section = 0;
    int seen_else = 0;
    int seen_fi = 0;

    for (int i = 1; i < nlines; i++) {
        char* l = lines[i];
        if (starts_with_word(l, "fi")) {
            seen_fi = 1;
            break;
        }
        if (starts_with_word(l, "then")) {
            section = 1;
            char* rest = (char*)trim_ws(l + 4);
            if (*rest != '\0' && nthen < 64) {
                then_lines[nthen++] = rest;
            }
            continue;
        }
        if (starts_with_word(l, "else")) {
            section = 2;
            seen_else = 1;
            char* rest = (char*)trim_ws(l + 4);
            if (*rest != '\0' && nelse < 64) {
                else_lines[nelse++] = rest;
            }
            continue;
        }
        if (section == 1 && nthen < 64) {
            then_lines[nthen++] = l;
        } else if (section == 2 && nelse < 64) {
            else_lines[nelse++] = l;
        }
    }

    (void)seen_else;
    int status = execute_line(condition);

    if (status == 0) {
        for (int i = 0; i < nthen; i++) {
            status = execute_line(then_lines[i]);
        }
    } else {
        for (int i = 0; i < nelse; i++) {
            status = execute_line(else_lines[i]);
        }
    }

    if (!seen_fi) {
        fprintf(stderr, "if: missing fi\n");
    }

    free(condition);
    free(copy);
    return status;
}

int execute_line(const char* line) {
    if (starts_with_word(line, "if")) {
        return execute_if_block(line);
    }

    char** tokens = tokenize(line);
    if (tokens == NULL) {
        return 0;
    }

    int status = 0;
    int pos = 0;

    while (tokens[pos] != NULL) {
        int seg_start = pos;
        int background = 0;

        for (int i = pos; tokens[i] != NULL &&
             strcmp(tokens[i], ";") != 0 && strcmp(tokens[i], "&") != 0; i++) {
            if (tokens[i][0] == '$' && tokens[i][1] != '\0') {
                const char* value = var_get(tokens[i] + 1);
                free(tokens[i]);
                tokens[i] = strdup(value != NULL ? value : "");
            }
        }

        Command cmds[MAXPIPE];
        int ncmds = parse_pipeline(tokens, &pos, cmds, &background);

        if (ncmds <= 0) {
            if (tokens[pos] != NULL) {
                pos++;
            }
            continue;
        }

        int empty = 0;
        for (int i = 0; i < ncmds; i++) {
            if (cmds[i].argc == 0) {
                empty = 1;
            }
        }
        if (empty) {
            fprintf(stderr, "Syntax error: empty command\n");
            status = 1;
            continue;
        }

        if (ncmds == 1 && !background && handle_builtin(cmds[0].args)) {
            status = 0;
        } else {
            int seg_end = pos;
            if (seg_end > seg_start &&
                (strcmp(tokens[seg_end - 1], "&") == 0 || strcmp(tokens[seg_end - 1], ";") == 0)) {
                seg_end--;
            }
            char* label = join_tokens(tokens, seg_start, seg_end);
            status = execute_pipeline(cmds, ncmds, background, label);
            free(label);
        }
    }

    free_tokens(tokens);
    return status;
}
