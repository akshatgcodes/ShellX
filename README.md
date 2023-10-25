# ShellX

A minimal Unix shell built from scratch in C. ShellX reads commands, parses
them, and executes them using POSIX system calls — `cd`, `pwd`, `ls`, `exit`,
and any external binary on your system.

```
$ ./shellx
shellx> pwd
/home/akshat
shellx> ls
projects  notes  Downloads
shellx> #projects          # fuzzy history search
shellx: recalled: cd projects
shellx> exit
```

## X Factor: command history with fuzzy search

ShellX ships with a built-in **command history with fuzzy search**, the
feature most mini-shell projects skip entirely:

- **Persistent, timestamped log.** Every non-empty command you run is
  appended to `~/.shellx_history.txt` as `[YYYY-MM-DD HH:MM:SS] <command>`,
  regardless of whether it was typed directly or recalled via `#keyword`.
  This file lives in your home directory, outside the git repo, so it is
  intentionally **not** gitignored by the project (there's nothing to
  ignore inside the repo — the file simply isn't there).

- **`#keyword` fuzzy search.** Typing `#keyword` at the prompt does **not**
  run `keyword` as a command. Instead ShellX scans
  `~/.shellx_history.txt` from oldest to newest, keeps track of the *last*
  (most recent) entry whose command text contains `keyword` as a
  substring, prints `shellx: recalled: <command>`, logs that command as a
  new history entry, and immediately executes it — exactly like bash's
  `!keyword` history expansion, but substring-based rather than
  prefix-based (hence "fuzzy": `#proj` matches `cd my-projects`, `#cd`
  matches the most recent command containing "cd" anywhere, etc.). If no
  entry matches, it prints `shellx: no history entry matching "keyword"`
  and does nothing further.

- **Up-arrow history scrolling.** ShellX links against the system's
  **GNU readline API** (`#include <readline/readline.h>` /
  `<readline/history.h>`, built with `-lreadline`) for line input instead
  of hand-rolling raw terminal mode. This gets real, battle-tested
  up/down-arrow history recall, left/right cursor movement, and line
  editing for free. On startup ShellX also replays every command
  previously logged in `~/.shellx_history.txt` into readline's in-memory
  history (stripping the `[timestamp]` prefix first), so pressing ↑ can
  recall commands from **previous sessions**, not just the current one.

  **Platform note:** on Linux this links against real GNU Readline. On
  macOS, the system ships **libedit**, a BSD-licensed library that
  implements a readline-compatible API and headers at the same
  `<readline/readline.h>` / `<readline/history.h>` paths — so
  `gcc shellx.c -o shellx -lreadline` links and works out of the box on
  macOS with **no extra installation** (verified on this machine: Apple
  clang, macOS SDK, `-lreadline` resolves to
  `/usr/lib/libedit.3.dylib`). If your system has neither libedit nor
  GNU readline installed, install readline yourself:
  - Debian/Ubuntu: `sudo apt-get install libreadline-dev`
  - Fedora: `sudo dnf install readline-devel`
  - macOS (only needed if you want real GNU readline instead of the
    bundled libedit): `brew install readline`, then compile with
    `gcc shellx.c -o shellx -Wall -I$(brew --prefix readline)/include -L$(brew --prefix readline)/lib -lreadline`

  I chose readline/libedit over hand-rolled raw-mode arrow key handling
  because it is dramatically less error-prone (correct handling of
  terminal modes, line redraw, cursor movement, and signal interaction
  is easy to get subtly wrong by hand) and it is verified working on
  this build machine — see the Testing section below.

## Key concepts demonstrated

- **`fork()` + `execvp()`** — every external command (anything that isn't
  `cd`, `pwd`, or `exit`) is run by forking a child process and calling
  `execvp()` in the child, so `$PATH` lookup works exactly like a real
  shell.
- **`waitpid()`** — the parent blocks on `waitpid()` for the child to
  finish before printing the next prompt, so commands run synchronously
  in the foreground.
- **Input parsing and tokenization** — `tokenize_line()` splits a raw
  input line into a `NULL`-terminated `argv[]` array on whitespace,
  growing its buffer dynamically for long command lines.
- **Command history with file persistence** — see the X Factor section
  above (`~/.shellx_history.txt`, `#keyword` search, readline history).
- **Signal handling (`SIGINT`)** — the shell installs its own `SIGINT`
  handler via `sigaction()` so Ctrl+C at the prompt never kills the shell
  process itself; readline aborts the line being edited and redisplays
  the prompt. When a foreground child is running, that same Ctrl+C is
  delivered to the child too (it shares the shell's process group), and
  because the child resets `SIGINT` to `SIG_DFL` immediately after
  `fork()` (before `execvp()`), the child is killed as expected while the
  shell keeps running and returns to the prompt once `waitpid()` reaps it.

## Build and run

Platform: Linux/macOS only (POSIX).

```sh
gcc shellx.c -o shellx -Wall -lreadline
./shellx
```

On macOS this compiles cleanly against the bundled libedit
readline-compatible headers/library with no extra flags or installation
(confirmed on this machine — Apple clang / macOS SDK). On Linux, install
`libreadline-dev` (or equivalent) first if it isn't already present.

Type `exit` (optionally `exit <code>`) or press Ctrl+D to quit.

## Testing performed

- Compiled with `gcc shellx.c -o shellx -Wall -lreadline`: zero warnings,
  zero errors.
- Scripted stdin session covering `pwd`, `ls`, `cd` into a real directory,
  an external command (`echo`), and `exit`: ran correctly with no crashes,
  and `~/.shellx_history.txt` was populated with correctly timestamped
  entries for each command.
- `#keyword` fuzzy search: after running several commands including
  `echo building shellx project` and `cd /tmp/shellx_test_projects`,
  `#project` correctly recalled and re-ran the most recent command
  containing "project"; `#tmp` correctly recalled and re-ran `cd /tmp`
  (verified the working directory actually changed afterward); a
  nonexistent keyword printed a clean "no history entry matching" message
  instead of crashing.
- `SIGINT` handling: using a pseudo-terminal harness, confirmed that (1)
  pressing Ctrl+C at an idle prompt does not exit the shell — it
  redisplays the prompt and the shell continues accepting commands, and
  (2) pressing Ctrl+C while a foreground child (`sleep 100`) is running
  kills the child (confirmed via `ps`) while the shell process survives
  and returns to the prompt, ready for the next command.

## Notes

Built as a focused, single-purpose tool - a minimal Unix shell built from scratch, nothing more, nothing less.

## Troubleshooting

If something doesn't run as expected, double-check you're using the dependency versions noted above and running the exact commands from the "Run it" section.

## Possible Improvements

- More test coverage
- Better error messages for edge cases
- A cleaner CLI/UI polish pass

## Acknowledgements

Thanks to the open-source libraries this project leans on - see the dependency list above for the full set.

## License

GPLv3 - see [LICENSE](LICENSE) for details.
