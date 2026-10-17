# Engineering Decisions — ShellX

---

## Decision 1 — readline / libedit instead of raw terminal mode

**Context:** ↑ / ↓ history and line editing need control of the terminal. The alternatives were hand-rolling raw mode with `termios`, or using a library.

**Decision:** Link `-lreadline`. On Linux that's GNU Readline. On macOS it resolves to the system's libedit, which provides the same `<readline/readline.h>` API.

**Reason:** Terminal modes, redraw, cursor movement and their interaction with signals are easy to get subtly wrong. The library is battle-tested, and it needs no extra install on macOS.

**Tradeoff:** One external dependency, and on Linux the `libreadline-dev` package must be installed to build.

---

## Decision 2 — One timestamped history file, replayed at startup

**Context:** History should survive the session and be readable on its own.

**Decision:** Append `[YYYY-MM-DD HH:MM:SS] cmd` to `~/.shellx_history.txt`. At startup, strip the timestamps and feed each command to `add_history()`.

**Reason:** The file doubles as an audit log, with when you ran what, and ↑ works across sessions with no separate format to keep in sync.

**Tradeoff:** The file only grows, and it's read in full for every `#keyword` search.

---

## Decision 3 — `#keyword` is a substring match, newest wins

**Context:** bash's `!prefix` only matches the start of a command, which is rarely what you remember.

**Decision:** Scan the history oldest to newest, keep the last line containing `keyword`, print what was recalled, log it again and run it.

**Reason:** "The last thing I did involving `proj`" is the common need. Echoing the recalled command means you always see what's about to run.

**Tradeoff:** It runs immediately with no confirmation. A loose keyword can run something you didn't expect, and `#` can't be used to start a real command.

---

## Decision 4 — The shell traps SIGINT, children don't

**Context:** Ctrl+C has to stop the running program but never the shell.

**Decision:** The shell installs a handler with `sigaction`. Children reset SIGINT and SIGQUIT to `SIG_DFL` between `fork` and `execvp`. The parent retries `waitpid` on `EINTR`.

**Reason:** It matches how real shells behave: the foreground job dies, the prompt comes back.

**Tradeoff:** No process groups or job control, so there's no way to background a job.

---

## Decision 5 — Builtins in a name → function table

**Context:** `cd` and `exit` must run inside the shell. A child process can't change the parent's directory or exit it.

**Decision:** Parallel arrays `builtin_names[]` and `builtin_funcs[]`, checked before falling through to `fork` / `execvp`.

**Reason:** Adding a builtin is one entry in each array, and the dispatch loop stays a few lines.

**Tradeoff:** A linear scan, which is irrelevant with three builtins.

---

## What I'd do differently in v2

- **A real tokenizer** with quotes, escapes, pipes and redirection.
- **A history size limit** and a way to skip logging, like a leading space.
- **Confirm before running** a `#keyword` recall, or add a `#keyword?` preview variant.

---

## Explicit non-decisions (deferred to v2)

| Feature | Why deferred |
|---|---|
| Job control (`&`, `fg`, `bg`) | Needs process groups and terminal ownership handling |
| Variable expansion | Needs a parser with quoting rules first |
| `.shellxrc` | Nothing to configure yet |

---

<div align="center">

<h3>Akshat Gupta</h3>

<a href="https://akshatgcodes.in"><img src="https://img.shields.io/badge/Website-akshatgcodes.in-111111?style=flat-square&logo=googlechrome&logoColor=white" alt="Website"></a>
<a href="https://linkedin.com/in/akshatgcodes"><img src="https://img.shields.io/badge/LinkedIn-akshatgcodes-0A66C2?style=flat-square" alt="LinkedIn"></a>
<a href="https://github.com/akshatgcodes"><img src="https://img.shields.io/badge/GitHub-akshatgcodes-181717?style=flat-square&logo=github&logoColor=white" alt="GitHub"></a>
<a href="mailto:akshatgcodes@gmail.com"><img src="https://img.shields.io/badge/Email-akshatgcodes%40gmail.com-EA4335?style=flat-square&logo=gmail&logoColor=white" alt="Email"></a>

</div>
