#include "shell.h"
#include <ctype.h>

static char* history[HISTORY_SIZE];
static int history_count = 0;

void history_add(const char* line) {
    if (line == NULL || line[0] == '\0') {
        return;
    }

    if (history_count < HISTORY_SIZE) {
        history[history_count++] = strdup(line);
        return;
    }

    free(history[0]);
    for (int i = 1; i < HISTORY_SIZE; i++) {
        history[i - 1] = history[i];
    }
    history[HISTORY_SIZE - 1] = strdup(line);
}

char* history_expand(const char* line) {
    const char* cp = line;
    while (*cp == ' ' || *cp == '\t') {
        cp++;
    }

    if (*cp != '!' || !isdigit((unsigned char)cp[1])) {
        return NULL;
    }

    int n = atoi(cp + 1);
    if (n < 1 || n > history_count) {
        fprintf(stderr, "history: %d: event not found\n", n);
        return strdup("");
    }

    return strdup(history[n - 1]);
}

void history_print(void) {
    for (int i = 0; i < history_count; i++) {
        printf("%3d  %s\n", i + 1, history[i]);
    }
}
