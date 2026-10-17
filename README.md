<p align="center">
  <svg xmlns="http://www.w3.org/2000/svg" width="64" height="64" viewBox="0 0 24 24"
       fill="none" stroke="#4ade80" stroke-width="1.5"
       stroke-linecap="round" stroke-linejoin="round">
    <polyline points="4 17 10 11 4 5"/>
    <line x1="12" x2="20" y1="19" y2="19"/>
  </svg>
</p>

<h1 align="center">ShellX</h1>

<p align="center">
  <strong>A minimal Unix shell in C with persistent, timestamped history and <code>#keyword</code> fuzzy recall.</strong>
</p>

<p align="center">
  <a href="https://shellx.akshatgcodes.in">
    <img src="https://img.shields.io/badge/project_page-4ade80-4ade80?style=flat-square" alt="Project page">
  </a>
  <img src="https://img.shields.io/badge/C-A8B9CC?style=flat-square&logo=c&logoColor=black" alt="C">
  <img src="https://img.shields.io/badge/POSIX-111111?style=flat-square" alt="POSIX">
  <img src="https://img.shields.io/badge/readline-111111?style=flat-square" alt="readline">
  <img src="https://img.shields.io/badge/license-GPL--3.0-4ade80?style=flat-square" alt="License">
</p>

---

## What is ShellX?

ShellX is a small shell written from scratch with POSIX system calls. It reads a line, splits it into arguments, and either runs a builtin (`cd`, `pwd`, `exit`) or forks and `execvp`s any program on your `PATH`. What most mini-shells skip is history: every command is logged with a timestamp to `~/.shellx_history.txt`, ↑ recalls commands from previous sessions, and `#keyword` finds and re-runs the most recent command containing that text.

> **Project page →** [shellx.akshatgcodes.in](https://shellx.akshatgcodes.in)
> It's a shell, so it runs on your machine: `gcc -Wall shellx.c -o shellx -lreadline && ./shellx`

```
shellx> cd ~/projects/portfolio
shellx> ls
shellx> #portf
shellx: recalled: cd ~/projects/portfolio
```

---

## What you get

- **Builtins + external commands** — `cd` (defaults to `$HOME`), `pwd` and `exit [code]` run in-process. Everything else goes through `fork()` → `execvp()` → `waitpid()`.
- **Ctrl+C that behaves** — it stops the running child, not the shell. The child gets default signal handling right after `fork()`.
- **Persistent history** — every command is appended as `[YYYY-MM-DD HH:MM:SS] cmd` and replayed into readline at startup, so ↑ works across sessions.
- **`#keyword` recall** — substring match against history, newest wins, then the command is logged again and run.

---

## Stack

| Layer | Tech |
|---|---|
| Language | C (POSIX: `fork`, `execvp`, `waitpid`, `sigaction`, `chdir`) |
| Line editing | GNU readline on Linux, or the readline-compatible libedit on macOS |
| Storage | `~/.shellx_history.txt` |
| Project page | Next.js 16 on Vercel · GoDaddy DNS |

---

## Engineering Decisions

**Why readline instead of hand-rolled raw terminal mode?**
Arrow keys, cursor movement, redraw and signal handling in raw mode are easy to get subtly wrong. readline (or libedit on macOS) gets them right, with no extra install on macOS.

**Why substring matching for `#keyword`?**
bash's `!prefix` only matches the start of a command. `#proj` matching `cd my-projects` is what you actually remember.

**Why a custom SIGINT handler in the shell?**
Ctrl+C should kill the foreground program, not the shell. The parent handles SIGINT, and the child restores the default handler before `execvp`.

**What would you do differently in v2?**
Add pipes (`|`), redirection (`>`, `<`) and quoted arguments. Today the tokenizer splits on whitespace only.

---

## Docs

| Document | Description |
|---|---|
| [PRD](docs/PRD.md) | Product requirements — goals, user stories, non-goals |
| [Architecture](docs/ARCHITECTURE.md) | REPL, process model, history design |
| [Decisions](docs/DECISIONS.md) | Every major technical decision and why |
| [Setup](docs/SETUP.md) | Build on Linux / macOS and usage |
| [Deploy](docs/DEPLOY.md) | How the project page is deployed |
| [Contributing](CONTRIBUTING.md) | How to propose changes |
| [Changelog](CHANGELOG.md) | What changed in each release |
| [Security](SECURITY.md) | How to report a vulnerability |

---

<div align="center">

<h3>Akshat Gupta</h3>

<a href="https://akshatgcodes.in"><img src="https://img.shields.io/badge/Website-akshatgcodes.in-111111?style=flat-square&logo=googlechrome&logoColor=white" alt="Website"></a>
<a href="https://linkedin.com/in/akshatgcodes"><img src="https://img.shields.io/badge/LinkedIn-akshatgcodes-0A66C2?style=flat-square" alt="LinkedIn"></a>
<a href="https://github.com/akshatgcodes"><img src="https://img.shields.io/badge/GitHub-akshatgcodes-181717?style=flat-square&logo=github&logoColor=white" alt="GitHub"></a>
<a href="mailto:akshatgcodes@gmail.com"><img src="https://img.shields.io/badge/Email-akshatgcodes%40gmail.com-EA4335?style=flat-square&logo=gmail&logoColor=white" alt="Email"></a>

</div>
