# ShellX — Product Requirements Document

**Status:** Final (v1 shipped)
**Owner:** Akshat Gupta
**One-liner:** A minimal Unix shell in C with persistent, timestamped history and `#keyword` fuzzy recall.

---

## 1. Problem

Most "build your own shell" projects stop at running a command. They lose history when you exit, have no arrow-key recall, and die when you press Ctrl+C. The useful part of a real shell is getting back to what you ran before, and that's exactly what gets skipped. This project is a small, readable shell that does the basics correctly and makes history a first-class feature.

---

## 2. Goals (v1)

1. A REPL with a `shellx> ` prompt that reads and runs commands until `exit` or EOF.
2. Builtins: `cd [dir]` (defaults to `$HOME`), `pwd`, `exit [code]`.
3. Run any program on `PATH` with `fork` + `execvp` + `waitpid`.
4. Ctrl+C interrupts the running program without killing the shell.
5. Log every command with a timestamp to `~/.shellx_history.txt`.
6. ↑ / ↓ recall across sessions, and `#keyword` to recall and re-run the newest matching command.

---

## 3. Non-Goals (explicit scope cuts)

- **Pipes and redirection** — `|`, `>`, `<` are passed through as plain arguments.
- **Quoting and escaping** — arguments are split on whitespace only.
- **Job control** — no `&`, `fg`, `bg` or `jobs`.
- **Variables and scripting** — no `$VAR` expansion, `if` or loops.

---

## 4. Users

**Primary:** Engineers learning (or reviewing) how a shell works at the system-call level.

**Secondary:** Anyone who wants a lightweight shell with better history recall than `!prefix`.

---

## 5. User Stories

1. *As a user,* I type `ls -la` so that it runs the system's `ls` with those arguments.
2. *As a user,* I type `cd` with no argument so that I go to my home directory.
3. *As a user,* I press Ctrl+C during `sleep 100` so that `sleep` stops and I get my prompt back.
4. *As a user,* I press ↑ in a new session so that I see commands from yesterday.
5. *As a user,* I type `#deploy` so that the last command containing "deploy" runs again.

---

## 6. Functional Requirements

### 6.1 REPL

- Leading and trailing whitespace is trimmed, and blank lines are ignored.
- Ctrl+D (EOF) exits cleanly.

### 6.2 Execution

- Builtins run in the shell process. Anything else is `fork`ed and `execvp`ed, and the parent waits.
- An unknown command prints `shellx: <cmd>: <error>` and the shell continues.
- `exit N` exits with status N.

### 6.3 Signals

- The shell installs a SIGINT handler so Ctrl+C at the prompt doesn't exit.
- Children reset SIGINT and SIGQUIT to their defaults before `execvp`, so Ctrl+C stops them.

### 6.4 History

- Each non-empty command is appended as `[YYYY-MM-DD HH:MM:SS] <command>`.
- At startup, the file is replayed into readline's history with the timestamps stripped.
- `#keyword` scans the file oldest to newest and keeps the last entry containing `keyword`. It prints `shellx: recalled: <cmd>`, logs it again and runs it. With no match, it prints `shellx: no history entry matching "keyword"`.

---

## 7. Non-Functional Requirements

- **Portability:** Linux with GNU readline, and macOS with the bundled libedit.
- **Robustness:** the tokenizer grows its buffer for long lines, and input lines are capped at 4,096 characters.
- **Clean build:** compiles with `-Wall -Wextra` (checked in CI on Ubuntu).

---

## 8. Success Metrics

| Metric | Target |
|---|---|
| Ctrl+C | Stops `sleep 100` and returns to the prompt every time |
| Cross-session recall | ↑ in a fresh session shows the previous session's commands |
| `#keyword` | Recalls the newest match, not the first |

---

## 9. Risks & Open Questions

- **History file growth** — it's never trimmed.
- **History privacy** — every command, including any secret typed on the command line, is written to a plain-text file in your home directory.
- **Signals during `waitpid`** — handled by retrying on `EINTR`, and worth retesting on every platform.

---

## 10. v2 Candidates

- Pipes and redirection.
- Quote-aware tokenizing.
- A history size limit and a way to skip logging (for example a leading space, like bash's `HISTCONTROL`).

---

<div align="center">

<h3>Akshat Gupta</h3>

<a href="https://akshatgcodes.in"><img src="https://img.shields.io/badge/Website-akshatgcodes.in-111111?style=flat-square&logo=googlechrome&logoColor=white" alt="Website"></a>
<a href="https://linkedin.com/in/akshatgcodes"><img src="https://img.shields.io/badge/LinkedIn-akshatgcodes-0A66C2?style=flat-square" alt="LinkedIn"></a>
<a href="https://github.com/akshatgcodes"><img src="https://img.shields.io/badge/GitHub-akshatgcodes-181717?style=flat-square&logo=github&logoColor=white" alt="GitHub"></a>
<a href="mailto:akshatgcodes@gmail.com"><img src="https://img.shields.io/badge/Email-akshatgcodes%40gmail.com-EA4335?style=flat-square&logo=gmail&logoColor=white" alt="Email"></a>

</div>
