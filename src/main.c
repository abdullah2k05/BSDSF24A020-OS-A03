#include "shell.h"
#include <ctype.h>

static const char* skip_ws(const char* s) {
    while (*s == ' ' || *s == '\t') {
        s++;
    }
    return s;
}

static int line_is(const char* s, const char* word) {
    s = skip_ws(s);
    int len = strlen(word);
    return strncmp(s, word, len) == 0 &&
           (s[len] == '\0' || isspace((unsigned char)s[len]));
}

static int ends_with_fi(const char* buf) {
    const char* last = strrchr(buf, '\n');
    last = (last != NULL) ? last + 1 : buf;
    return line_is(last, "fi");
}

static char* read_if_block(char* first) {
    if (!line_is(first, "if")) {
        return first;
    }

    char* buf = strdup(first);
    while (!ends_with_fi(buf)) {
        char* line = read_cmd("> ");
        if (line == NULL) {
            break;
        }
        size_t oldlen = strlen(buf);
        buf = realloc(buf, oldlen + strlen(line) + 2);
        buf[oldlen] = '\n';
        strcpy(buf + oldlen + 1, line);
        free(line);
    }
    return buf;
}

int main(void) {
    char* cmdline;

    while ((cmdline = read_cmd(PROMPT)) != NULL) {
        if (cmdline[0] == '\0') {
            free(cmdline);
            continue;
        }

        char* expanded = shell_history_expand(cmdline);
        if (expanded != NULL) {
            free(cmdline);
            cmdline = expanded;
        }
        cmdline = read_if_block(cmdline);
        history_add(cmdline);

        jobs_reap();
        execute_line(cmdline);
        jobs_reap();
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
