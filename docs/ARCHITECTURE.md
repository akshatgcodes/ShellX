# ShellX — Architecture

---

## 1. Stack

| Layer | Tech |
|---|---|
| Language | C, POSIX APIs |
| Process control | `fork`, `execvp`, `waitpid` |
| Signals | `sigaction` for the shell, `signal(SIG_DFL)` in children |
| Line editing | `readline()` / `add_history()`: GNU readline (Linux) or libedit (macOS) |
| Storage | `~/.shellx_history.txt` |
| Project page | Next.js 16 static site in `landing/`, on Vercel |

---

## 2. Components

```
shellx.c   the whole shell: history, tokenizer, signals, builtins, launcher, REPL
landing/   Next.js project page (separate from the shell)
```

| Function | Role |
|---|---|
| `init_history_path()` | Builds `$HOME/.shellx_history.txt` once at startup |
| `log_history(line)` | Appends `[timestamp] line` |
| `load_history_file()` | Replays saved commands into readline, without timestamps |
| `find_history_match(keyword)` | Returns the newest logged command containing `keyword` |
| `tokenize_line(line)` | Whitespace split into a `NULL`-terminated `argv`, growing in steps of 64 |
| `setup_signal_handling()` | Installs the shell's SIGINT handler |
| `builtin_cd/pwd/exit` | In-process builtins, looked up in a name → function table |
| `launch_external(args)` | `fork`; the child resets signals and `execvp`s; the parent `waitpid`s |
| `run_line()` / `handle_history_search()` | Dispatch a line or a `#keyword` |
| `main()` | `readline("shellx> ")` loop until EOF |

---

## 3. Data Flow

```
readline("shellx> ")
  └─ trim; empty? → next prompt
  └─ starts with '#'?
        find_history_match(keyword)
          none  → "no history entry matching"
          found → print "recalled: cmd" → log_history(cmd) → run_line(cmd)
  └─ else
        add_history(line); log_history(line); run_line(line)

run_line(line)
  └─ tokenize_line → argv
  └─ argv[0] in {cd, pwd, exit}? → builtin
  └─ else launch_external(argv)
        fork ─┬─ child:  SIGINT/SIGQUIT = default → execvp(argv[0], argv)
              └─ parent: waitpid(child), retrying on EINTR
```

---

## 4. History File

```
[2023-10-14 21:03:11] cd ~/projects
[2023-10-14 21:03:15] ls -la
[2023-10-14 21:04:02] cd ~/projects
```

- Append-only, one line per command, including commands recalled with `#keyword`.
- At startup the `[...]` prefix is stripped and each command is passed to `add_history`.
- `#keyword` reads the whole file each time and keeps the last match.

---

## 5. Signals

1. The shell installs a SIGINT handler, so Ctrl+C at the prompt doesn't exit.
2. While a child runs, Ctrl+C reaches the child too, because it's in the same foreground process group.
3. Right after `fork()`, the child resets SIGINT and SIGQUIT to `SIG_DFL`, so it dies on Ctrl+C like it would in bash.
4. The parent's `waitpid` is retried on `EINTR`, so its own handler firing doesn't break the wait.

---

## 6. Security

- **No shell expansion:** arguments go straight to `execvp`. There's no `system()` or `/bin/sh -c`, so no injection through expansion.
- **Bounded input:** history lines are read into 4,096-byte buffers, and the token array grows as needed.
- **History contents:** commands are stored in plain text with your normal file permissions. Don't type secrets as arguments.

---

## 7. Error Handling

| Failure | Behaviour |
|---|---|
| Unknown program | Child prints `shellx: <cmd>: <strerror>` and exits; the shell continues |
| `fork` fails | `shellx: fork failed: …` |
| `cd` to a bad path | `shellx: cd: <dir>: <strerror>` |
| `cd` with no `$HOME` | `shellx: cd: HOME not set` |
| No history match | `shellx: no history entry matching "<keyword>"` |

---

## 8. Deployment

The shell isn't hosted. It's built and run locally. The project page in `landing/` is a Vercel project with root directory `landing`, deployed on every push to `master` (see [DEPLOY.md](DEPLOY.md)).

---

## 9. Explicit Scope Cuts

- **Pipes / redirection / quoting** — the tokenizer is whitespace-only.
- **Job control** — everything runs in the foreground.
- **Config files** — no `.shellxrc`.

---

<div align="center">

<h3>Akshat Gupta</h3>

<a href="https://akshatgcodes.in"><img src="https://img.shields.io/badge/Website-akshatgcodes.in-111111?style=flat-square&logo=googlechrome&logoColor=white" alt="Website"></a>
<a href="https://linkedin.com/in/akshatgcodes"><img src="https://img.shields.io/badge/LinkedIn-akshatgcodes-0A66C2?style=flat-square" alt="LinkedIn"></a>
<a href="https://github.com/akshatgcodes"><img src="https://img.shields.io/badge/GitHub-akshatgcodes-181717?style=flat-square&logo=github&logoColor=white" alt="GitHub"></a>
<a href="mailto:akshatgcodes@gmail.com"><img src="https://img.shields.io/badge/Email-akshatgcodes%40gmail.com-EA4335?style=flat-square&logo=gmail&logoColor=white" alt="Email"></a>

</div>
