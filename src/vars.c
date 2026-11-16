#include "shell.h"
#include <ctype.h>

typedef struct Var {
    char* name;
    char* value;
    struct Var* next;
} Var;

static Var* variables = NULL;

const char* var_get(const char* name) {
    for (Var* v = variables; v != NULL; v = v->next) {
        if (strcmp(v->name, name) == 0) {
            return v->value;
        }
    }
    return NULL;
}

static void var_set(const char* name, const char* value) {
    for (Var* v = variables; v != NULL; v = v->next) {
        if (strcmp(v->name, name) == 0) {
            free(v->value);
            v->value = strdup(value);
            return;
        }
    }

    Var* v = malloc(sizeof(Var));
    v->name = strdup(name);
    v->value = strdup(value);
    v->next = variables;
    variables = v;
}

int is_assignment(const char* token) {
    const char* eq = strchr(token, '=');
    if (eq == NULL || eq == token) {
        return 0;
    }
    if (!(isalpha((unsigned char)token[0]) || token[0] == '_')) {
        return 0;
    }
    for (const char* p = token; p < eq; p++) {
        if (!(isalnum((unsigned char)*p) || *p == '_')) {
            return 0;
        }
    }
    return 1;
}

int assign_variable(const char* token) {
    if (!is_assignment(token)) {
        return 0;
    }

    const char* eq = strchr(token, '=');
    size_t name_len = eq - token;
    char name[128];
    if (name_len >= sizeof(name)) {
        return 0;
    }
    memcpy(name, token, name_len);
    name[name_len] = '\0';

    const char* value = eq + 1;
    if (value[0] == '$') {
        const char* referenced = var_get(value + 1);
        value = referenced != NULL ? referenced : "";
    }
    var_set(name, value);
    return 1;
}

void vars_print(void) {
    for (Var* v = variables; v != NULL; v = v->next) {
        printf("%s=%s\n", v->name, v->value);
    }
}
