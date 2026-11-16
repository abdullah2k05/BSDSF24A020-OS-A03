#include "shell.h"

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
        history_add(cmdline);

        execute_line(cmdline);
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
