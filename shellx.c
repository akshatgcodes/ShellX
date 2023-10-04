/*
 * ShellX - a minimal Unix shell built from scratch in C.
 *
 * Implements:
 *   - a REPL that reads, tokenizes, and executes commands
 *   - builtins: cd, pwd, exit
 *   - external commands via fork() + execvp() + waitpid()
 *
 * Build:
 *   gcc shellx.c -o shellx -Wall
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>

#define SHELLX_TOK_BUFSIZE 64
#define SHELLX_TOK_DELIM " \t\r\n\a"
#define SHELLX_MAX_LINE 4096

/* ------------------------------------------------------------------ */
/* Parsing                                                             */
/* ------------------------------------------------------------------ */

/* Split a line into a NULL-terminated argv array on whitespace. */
static char **tokenize_line(const char *line) {
    int bufsize = SHELLX_TOK_BUFSIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *line_copy = strdup(line);
    char *token;

    if (!tokens || !line_copy) {
        fprintf(stderr, "shellx: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line_copy, SHELLX_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = strdup(token);
        position++;

        if (position >= bufsize) {
            bufsize += SHELLX_TOK_BUFSIZE;
            char **new_tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!new_tokens) {
                fprintf(stderr, "shellx: allocation error\n");
                exit(EXIT_FAILURE);
            }
            tokens = new_tokens;
        }
        token = strtok(NULL, SHELLX_TOK_DELIM);
    }
    tokens[position] = NULL;
    free(line_copy);
    return tokens;
}

static void free_tokens(char **tokens) {
    if (!tokens) return;
    for (int i = 0; tokens[i] != NULL; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

/* ------------------------------------------------------------------ */
/* Builtins                                                            */
/* ------------------------------------------------------------------ */

static int builtin_cd(char **args) {
    const char *target = args[1];
    if (target == NULL) {
        target = getenv("HOME");
        if (target == NULL) {
            fprintf(stderr, "shellx: cd: HOME not set\n");
            return 1;
        }
    }
    if (chdir(target) != 0) {
        fprintf(stderr, "shellx: cd: %s: %s\n", target, strerror(errno));
        return 1;
    }
    return 1;
}

static int builtin_pwd(char **args) {
    (void)args;
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        fprintf(stderr, "shellx: pwd: %s\n", strerror(errno));
    }
    return 1;
}

static int builtin_exit(char **args) {
    int code = 0;
    if (args[1] != NULL) {
        code = atoi(args[1]);
    }
    exit(code);
    return 0; /* unreachable */
}

typedef int (*builtin_fn)(char **);

static const char *builtin_names[] = { "cd", "pwd", "exit" };
static builtin_fn builtin_funcs[] = { builtin_cd, builtin_pwd, builtin_exit };
static const int num_builtins = sizeof(builtin_names) / sizeof(char *);

/* ------------------------------------------------------------------ */
/* Execution                                                           */
/* ------------------------------------------------------------------ */

/* Fork + exec an external command, waiting for it to finish.
 * Returns 1 to keep the REPL going. */
static int launch_external(char **args) {
    pid_t pid = fork();

    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            fprintf(stderr, "shellx: %s: %s\n", args[0], strerror(errno));
        }
        _exit(EXIT_FAILURE);
    } else if (pid < 0) {
        fprintf(stderr, "shellx: fork failed: %s\n", strerror(errno));
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
    return 1;
}

/* Dispatch a single already-tokenized command: builtin or external. */
static int execute(char **args) {
    if (args[0] == NULL) {
        return 1;
    }

    for (int i = 0; i < num_builtins; i++) {
        if (strcmp(args[0], builtin_names[i]) == 0) {
            return builtin_funcs[i](args);
        }
    }

    return launch_external(args);
}

/* ------------------------------------------------------------------ */
/* Main loop                                                           */
/* ------------------------------------------------------------------ */

int main(void) {
    printf("ShellX - a minimal Unix shell. Type 'exit' to quit.\n");

    char line[SHELLX_MAX_LINE];
    for (;;) {
        printf("shellx> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[len - 1] = '\0';
            len--;
        }

        if (line[0] == '\0') continue;

        char **args = tokenize_line(line);
        execute(args);
        free_tokens(args);
    }

    return 0;
}
