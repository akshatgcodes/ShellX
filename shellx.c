/*
 * ShellX - a minimal Unix shell built from scratch in C.
 *
 * Implements:
 *   - a REPL that reads, tokenizes, and executes commands
 *   - builtins: cd, pwd, exit
 *   - external commands via fork() + execvp() + waitpid()
 *   - SIGINT handling so Ctrl+C does not kill the shell itself
 *   - persistent command history with timestamps in ~/.shellx_history.txt
 *   - "#keyword" fuzzy history search
 *   - up-arrow history scrolling via GNU readline (or macOS's
 *     readline-compatible libedit, whichever the system provides)
 *
 * Build:
 *   gcc shellx.c -o shellx -Wall -lreadline
 *
 * See README.md for full details and design rationale.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>

#include <readline/readline.h>
#include <readline/history.h>

#define SHELLX_TOK_BUFSIZE 64
#define SHELLX_TOK_DELIM " \t\r\n\a"
#define SHELLX_MAX_LINE 4096

static char history_path[PATH_MAX];
static volatile pid_t foreground_child = -1;

/* ------------------------------------------------------------------ */
/* History: persistent, timestamped, file-backed                      */
/* ------------------------------------------------------------------ */

/* Build the path to ~/.shellx_history.txt once at startup. */
static void init_history_path(void) {
    const char *home = getenv("HOME");
    if (!home) home = ".";
    snprintf(history_path, sizeof(history_path), "%s/.shellx_history.txt", home);
}

/* Append a single command line to the history file with a timestamp,
 * and register it with readline so it is recalled with the up arrow
 * for the rest of this session. Blank lines are not logged. */
static void log_history(const char *line) {
    if (!line || line[0] == '\0') return;

    /* Skip logging pure whitespace. */
    const char *p = line;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p == '\0') return;

    add_history(line);

    FILE *f = fopen(history_path, "a");
    if (!f) {
        /* Non-fatal: history logging failing shouldn't crash the shell. */
        return;
    }

    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm_now);

    fprintf(f, "[%s] %s\n", timestamp, line);
    fclose(f);
}

/* Load ~/.shellx_history.txt at startup and feed each previously logged
 * command into readline's in-memory history, so the up arrow can recall
 * commands from earlier sessions too. */
static void load_history_file(void) {
    FILE *f = fopen(history_path, "r");
    if (!f) return;

    char line[SHELLX_MAX_LINE];
    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';

        /* Strip the leading "[timestamp] " prefix, if present, before
         * handing the bare command to readline's history. */
        char *cmd = line;
        if (line[0] == '[') {
            char *close = strchr(line, ']');
            if (close && *(close + 1) == ' ') {
                cmd = close + 2;
            }
        }
        if (cmd[0] != '\0') {
            add_history(cmd);
        }
    }
    fclose(f);
}

/* "#keyword" fuzzy search: scan ~/.shellx_history.txt from the bottom
 * (most recent first) and return the most recent command whose text
 * contains "keyword" as a substring. Returns a newly malloc'd string
 * (caller must free) or NULL if nothing matched. */
static char *find_history_match(const char *keyword) {
    FILE *f = fopen(history_path, "r");
    if (!f) return NULL;

    /* The history file is append-only and can grow, but for a learning
     * shell it's small enough to just read every line and remember the
     * last match seen; simple and correct. */
    char line[SHELLX_MAX_LINE];
    char *match = NULL;

    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';

        char *cmd = line;
        if (line[0] == '[') {
            char *close = strchr(line, ']');
            if (close && *(close + 1) == ' ') {
                cmd = close + 2;
            }
        }
        if (cmd[0] == '\0') continue;

        if (strstr(cmd, keyword) != NULL) {
            free(match);
            match = strdup(cmd);
        }
    }
    fclose(f);
    return match;
}

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
 * the signal and let readline redraw the prompt on a fresh line -
 * the shell process is never killed by Ctrl+C.
 *
 * When a foreground child IS running, the terminal's Ctrl+C also
 * delivers SIGINT to that child directly (it's in the same process
 * group / foreground), and because the child resets SIGINT to its
 * default disposition right after fork(), the child dies as expected
 * while the shell's own handler here just no-ops for the parent.
 */
static void sigint_handler(int signo) {
    (void)signo;
    /* Nothing to do for the parent process itself. readline aborts the
     * line currently being edited and redisplays the prompt on a new
     * line once this handler returns, so the shell simply keeps going. */
    if (foreground_child == -1) {
        /* No child running: move to a fresh line so the next prompt
         * doesn't get printed in the middle of whatever the user had
         * typed. readline redraws the prompt right after this. */
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
        /* Child: restore default SIGINT/SIGQUIT behavior so Ctrl+C
         * kills a runaway foreground command instead of being
         * swallowed like it is in the shell itself. */
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);

        if (execvp(args[0], args) == -1) {
            fprintf(stderr, "shellx: %s: %s\n", args[0], strerror(errno));
        }
        _exit(EXIT_FAILURE);
    } else if (pid < 0) {
        fprintf(stderr, "shellx: fork failed: %s\n", strerror(errno));
    } else {
        /* Parent: remember the child so we know one is in the
         * foreground, then wait for it, tolerating EINTR from our
         * own SIGINT handler firing while we wait. */
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
        /* Empty command. */
        return 1;
    }

    for (int i = 0; i < num_builtins; i++) {
        if (strcmp(args[0], builtin_names[i]) == 0) {
            return builtin_funcs[i](args);
        }
    }

    return launch_external(args);
}

/* Run a raw command line: tokenize then execute. Shared by normal
 * input and by "#keyword" recall. */
static void run_line(const char *line) {
    char **args = tokenize_line(line);
    execute(args);
    free_tokens(args);
}

/* Handle a line starting with '#': treat the rest as a fuzzy keyword,
 * look up the most recent matching history entry, print what was
 * recalled, log + run it. If nothing matches, say so and do nothing. */
static void handle_history_search(const char *line) {
    const char *keyword = line + 1; /* skip '#' */
    while (*keyword && isspace((unsigned char)*keyword)) keyword++;

    if (*keyword == '\0') {
        fprintf(stderr, "shellx: #<keyword> requires a keyword, e.g. #proj\n");
        return;
    }

    char *match = find_history_match(keyword);
    if (!match) {
        printf("shellx: no history entry matching \"%s\"\n", keyword);
        return;
    }

    printf("shellx: recalled: %s\n", match);
    log_history(match);
    run_line(match);
    free(match);
}

/* ------------------------------------------------------------------ */
/* Main loop                                                           */
/* ------------------------------------------------------------------ */

int main(void) {
    init_history_path();
    setup_signal_handling();

    load_history_file();

    printf("ShellX - a minimal Unix shell. Type 'exit' to quit.\n");

    char *line;
    while ((line = readline("shellx> ")) != NULL) {
        /* Trim leading/trailing whitespace for cleaner matching/logging. */
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

        if (start[0] == '#') {
            /* Fuzzy history search/recall - not logged as a raw
             * "#keyword" entry itself; the recalled command is logged
             * inside handle_history_search(). */
            add_history(start);
            handle_history_search(start);
        } else {
            log_history(start);
            run_line(start);
        }

        free(line);
    }

    /* EOF (Ctrl+D) on the input stream. */
    printf("\n");
    return 0;
}

// Built incrementally - see git history for the development progression.
