#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;

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

        if ((arglist = tokenize(cmdline)) != NULL) {
            if (handle_builtin(arglist) == 0) {
                execute(arglist);
            }

            // Free the memory allocated by tokenize()
            for (int i = 0; arglist[i] != NULL; i++) {
                free(arglist[i]);
            }
            free(arglist);
        }
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}