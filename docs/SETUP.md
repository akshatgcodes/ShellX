# Local Setup — ShellX

> **Project page:** [shellx.akshatgcodes.in](https://shellx.akshatgcodes.in)
> ShellX is a shell, so it runs on your machine. This guide covers building and using it.

---

## Prerequisites

- A C compiler (GCC or Clang)
- readline headers:
  - **macOS:** nothing to install. `-lreadline` links against the bundled libedit
  - **Debian / Ubuntu:** `sudo apt-get install libreadline-dev`
  - **Fedora:** `sudo dnf install readline-devel`

---

## 1. Clone and build

```bash
git clone https://github.com/akshatgcodes/ShellX
cd ShellX
gcc -Wall shellx.c -o shellx -lreadline
```

To use real GNU readline on macOS instead of libedit:

```bash
brew install readline
gcc shellx.c -o shellx -Wall -I$(brew --prefix readline)/include -L$(brew --prefix readline)/lib -lreadline
```

---

## 2. Environment variables

None. ShellX uses `$HOME` to find `~/.shellx_history.txt` and as the default for `cd`.

---

## 3. Use it

```bash
./shellx
```

```
shellx> pwd
shellx> cd /tmp
shellx> ls -la
shellx> sleep 100        # press Ctrl+C: sleep stops, the shell stays
shellx> #tmp             # re-runs the newest command containing "tmp"
shellx: recalled: cd /tmp
shellx> exit
```

- **↑ / ↓** scroll through history, including previous sessions.
- **`#keyword`** finds the newest command containing `keyword`, prints it and runs it.
- **`exit [code]`** or **Ctrl+D** quits.

---

## 4. Project page (optional)

```bash
cd landing && npm install && npm run dev   # http://localhost:3000
```

See [DEPLOY.md](DEPLOY.md) for how it's hosted.

---

## Testing

There's no automated test suite yet. CI compiles `shellx.c` with `-Wall -Wextra` on Ubuntu and lints and builds the project page. To check a change by hand:

1. Run `sleep 100` and press Ctrl+C. The prompt comes back.
2. Run a few commands, `exit`, start again, and press ↑. Last session's commands appear.
3. Run `#` plus part of an earlier command. The newest match runs.
4. Run `nosuchcommand`. You get an error and the shell keeps going.

---

## Known limitations

- No pipes, redirection or quotes. `echo "a b"` passes `"a` and `b"` as two arguments.
- Every command you type is saved in plain text in `~/.shellx_history.txt`.

---

<div align="center">

<h3>Akshat Gupta</h3>

<a href="https://akshatgcodes.in"><img src="https://img.shields.io/badge/Website-akshatgcodes.in-111111?style=flat-square&logo=googlechrome&logoColor=white" alt="Website"></a>
<a href="https://linkedin.com/in/akshatgcodes"><img src="https://img.shields.io/badge/LinkedIn-akshatgcodes-0A66C2?style=flat-square" alt="LinkedIn"></a>
<a href="https://github.com/akshatgcodes"><img src="https://img.shields.io/badge/GitHub-akshatgcodes-181717?style=flat-square&logo=github&logoColor=white" alt="GitHub"></a>
<a href="mailto:akshatgcodes@gmail.com"><img src="https://img.shields.io/badge/Email-akshatgcodes%40gmail.com-EA4335?style=flat-square&logo=gmail&logoColor=white" alt="Email"></a>

</div>
