/*
 * ShellX - a minimal Unix shell built from scratch in C.
 *
 * Implements:
 *   - a REPL that reads, tokenizes, and executes commands
 *   - builtins: cd, pwd, exit
 *   - external commands via fork() + execvp() + waitpid()
 *   - SIGINT handling so Ctrl+C does not kill the shell itself
 *   - up-arrow history scrolling via GNU readline (or macOS's
 *     readline-compatible libedit, whichever the system provides)
 *
 * Build:
 *   gcc shellx.c -o shellx -Wall -lreadline
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>

#include <readline/readline.h>
#include <readline/history.h>

#define SHELLX_TOK_BUFSIZE 64
#define SHELLX_TOK_DELIM " \t\r\n\a"
#define SHELLX_MAX_LINE 4096

static volatile pid_t foreground_child = -1;

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
/* Signal handling                                                     */
/* ------------------------------------------------------------------ */

/* SIGINT handler for the shell process itself. When Ctrl+C is pressed
 * at an empty prompt (no foreground child running), we simply swallow
 * the signal and let readline redraw the prompt on a fresh line - the
 * shell process is never killed by Ctrl+C. When a foreground child is
 * running, the terminal's Ctrl+C also delivers SIGINT to that child
 * directly, and because the child resets SIGINT to its default
 * disposition right after fork(), the child dies as expected while
 * the shell's own handler here just no-ops for the parent.
 */
static void sigint_handler(int signo) {
    (void)signo;
    if (foreground_child == -1) {
        write(STDOUT_FILENO, "\n", 1);
    }
}

static void setup_signal_handling(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);
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
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);

        if (execvp(args[0], args) == -1) {
            fprintf(stderr, "shellx: %s: %s\n", args[0], strerror(errno));
        }
        _exit(EXIT_FAILURE);
    } else if (pid < 0) {
        fprintf(stderr, "shellx: fork failed: %s\n", strerror(errno));
    } else {
        foreground_child = pid;
        int status;
        pid_t w;
        do {
            w = waitpid(pid, &status, 0);
        } while (w == -1 && errno == EINTR);
        foreground_child = -1;
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
    setup_signal_handling();

    printf("ShellX - a minimal Unix shell. Type 'exit' to quit.\n");

    char *line;
    while ((line = readline("shellx> ")) != NULL) {
        char *start = line;
        while (*start && isspace((unsigned char)*start)) start++;
        size_t len = strlen(start);
        while (len > 0 && isspace((unsigned char)start[len - 1])) {
            start[len - 1] = '\0';
            len--;
        }

        if (start[0] == '\0') {
            free(line);
            continue;
        }

        add_history(start);
        char **args = tokenize_line(start);
        execute(args);
        free_tokens(args);

        free(line);
    }

    printf("\n");
    return 0;
}
