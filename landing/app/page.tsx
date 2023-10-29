import Terminal from "@/app/components/Terminal";
import Badge from "@/app/components/Badge";

const CONCEPTS = [
  "fork()",
  "execvp()",
  "waitpid()",
  "POSIX",
  "C99",
  "tokenizer",
  "GNU readline",
  "SIGINT handler",
  "history persistence",
];

const FEATURES = [
  {
    title: "Fuzzy #keyword history search",
    body: "Type # followed by any fragment of a past command and ShellX scans its history for the most recent match — no scrolling, no retyping. #proj instantly recalls cd projects from ten commands ago.",
  },
  {
    title: "Arrow-key recall, via readline",
    body: "ShellX links against GNU readline, so the up and down arrows walk backward and forward through everything you have typed in the session, with normal line-editing (Ctrl+A/E, word-delete) for free.",
  },
  {
    title: "Every command, timestamped, on disk",
    body: "Each line you run is appended to ~/.shellx_history.txt with a timestamp the moment it executes, so your shell history survives restarts and doubles as an audit log.",
  },
  {
    title: "Ctrl+C that respects the shell",
    body: "ShellX installs its own SIGINT handler. Pressing Ctrl+C interrupts whatever child process is running in the foreground — it never kills the shell itself, matching how bash and zsh behave.",
  },
];

export default function Home() {
  return (
    <div className="flex flex-1 flex-col">
      <header className="sticky top-0 z-10 border-b border-border bg-background/80 backdrop-blur">
        <div className="mx-auto flex w-full max-w-5xl items-center justify-between px-6 py-4">
          <span className="font-mono text-sm font-semibold tracking-tight text-foreground">
            shellx<span className="text-accent">_</span>
          </span>
          <nav className="flex items-center gap-5 font-mono text-xs text-muted">
            <a href="#x-factor" className="transition-colors hover:text-accent">
              x-factor
            </a>
            <a href="#run-it" className="transition-colors hover:text-accent">
              run it
            </a>
            <span className="rounded-full border border-border px-2.5 py-1 text-muted">
              GPLv3
            </span>
          </nav>
        </div>
      </header>

      <main className="flex-1">
        {/* Hero */}
        <section className="mx-auto flex w-full max-w-5xl flex-col items-start gap-10 px-6 pb-20 pt-16 sm:pt-24">
          <div className="flex flex-col gap-5">
            <span className="font-mono text-xs uppercase tracking-[0.2em] text-accent">
              a unix shell, built from scratch
            </span>
            <h1 className="glow-text max-w-2xl text-4xl font-semibold tracking-tight text-foreground sm:text-5xl">
              ShellX
            </h1>
            <p className="max-w-xl text-lg leading-8 text-muted">
              A minimal POSIX shell written from first principles in C —
              spawning and waiting on real processes with{" "}
              <code className="rounded bg-surface px-1.5 py-0.5 font-mono text-[0.9em] text-foreground">
                fork()
              </code>
              ,{" "}
              <code className="rounded bg-surface px-1.5 py-0.5 font-mono text-[0.9em] text-foreground">
                exec()
              </code>{" "}
              and{" "}
              <code className="rounded bg-surface px-1.5 py-0.5 font-mono text-[0.9em] text-foreground">
                waitpid()
              </code>
              . Built-in <code className="text-foreground">cd</code>,{" "}
              <code className="text-foreground">pwd</code> and{" "}
              <code className="text-foreground">exit</code>, plus anything
              else on your{" "}
              <code className="rounded bg-surface px-1.5 py-0.5 font-mono text-[0.9em] text-foreground">
                $PATH
              </code>
              .
            </p>
            <div className="flex flex-wrap gap-3 pt-2">
              <a
                href="#x-factor"
                className="rounded-md bg-accent px-4 py-2 font-mono text-sm font-medium text-[#05130b] transition-colors hover:bg-[#5eea94]"
              >
                See the X factor
              </a>
              <a
                href="#run-it"
                className="rounded-md border border-border px-4 py-2 font-mono text-sm text-foreground transition-colors hover:border-accent-dim hover:text-accent"
              >
                gcc shellx.c -o shellx -lreadline
              </a>
            </div>
          </div>

          <Terminal />
        </section>

        {/* X Factor */}
        <section
          id="x-factor"
          className="border-t border-border bg-surface/40 px-6 py-20"
        >
          <div className="mx-auto w-full max-w-5xl">
            <span className="font-mono text-xs uppercase tracking-[0.2em] text-amber">
              the x factor
            </span>
            <h2 className="mt-3 max-w-xl text-3xl font-semibold tracking-tight text-foreground">
              More than fork-and-exec: a shell that remembers.
            </h2>
            <p className="mt-4 max-w-2xl leading-7 text-muted">
              Most from-scratch shells stop once a command runs. ShellX also
              treats history as a first-class feature — searchable, durable,
              and interruptible without ever taking the shell down with it.
            </p>

            <div className="mt-10 grid grid-cols-1 gap-5 sm:grid-cols-2">
              {FEATURES.map((f) => (
                <div
                  key={f.title}
                  className="rounded-lg border border-border bg-surface p-6"
                >
                  <h3 className="font-mono text-sm font-semibold text-accent">
                    {f.title}
                  </h3>
                  <p className="mt-3 text-sm leading-6 text-muted">
                    {f.body}
                  </p>
                </div>
              ))}
            </div>
          </div>
        </section>

        {/* Key concepts */}
        <section className="border-t border-border px-6 py-16">
          <div className="mx-auto w-full max-w-5xl">
            <span className="font-mono text-xs uppercase tracking-[0.2em] text-muted">
              key concepts
            </span>
            <div className="mt-5 flex flex-wrap gap-2.5">
              {CONCEPTS.map((c) => (
                <Badge key={c}>{c}</Badge>
              ))}
            </div>
          </div>
        </section>

        {/* Run it */}
        <section
          id="run-it"
          className="border-t border-border bg-surface/40 px-6 py-20"
        >
          <div className="mx-auto flex w-full max-w-5xl flex-col gap-6">
            <span className="font-mono text-xs uppercase tracking-[0.2em] text-accent">
              run it
            </span>
            <h2 className="max-w-xl text-3xl font-semibold tracking-tight text-foreground">
              One file, one compiler flag.
            </h2>

            <div className="mt-2 overflow-x-auto rounded-lg border border-border bg-[#050607] p-5 font-mono text-sm">
              <div className="text-muted"># build</div>
              <div className="text-foreground">
                <span className="text-accent">$</span> gcc shellx.c -o shellx
                -lreadline
              </div>
              <div className="mt-3 text-muted"># run</div>
              <div className="text-foreground">
                <span className="text-accent">$</span> ./shellx
              </div>
            </div>

            <div className="grid grid-cols-1 gap-5 sm:grid-cols-2">
              <div className="rounded-lg border border-border bg-surface p-6">
                <h3 className="font-mono text-sm font-semibold text-foreground">
                  Dependency: readline
                </h3>
                <p className="mt-3 text-sm leading-6 text-muted">
                  ShellX links against GNU readline for arrow-key history
                  recall and line editing.
                </p>
                <div className="mt-4 space-y-1.5 font-mono text-xs text-muted">
                  <div>
                    <span className="text-accent">macOS</span> brew install
                    readline
                  </div>
                  <div>
                    <span className="text-accent">Debian/Ubuntu</span> apt
                    install libreadline-dev
                  </div>
                </div>
              </div>
              <div className="rounded-lg border border-border bg-surface p-6">
                <h3 className="font-mono text-sm font-semibold text-foreground">
                  Platform
                </h3>
                <p className="mt-3 text-sm leading-6 text-muted">
                  Built on POSIX APIs (
                  <code className="text-foreground">fork</code>,{" "}
                  <code className="text-foreground">exec</code>,{" "}
                  <code className="text-foreground">waitpid</code>,{" "}
                  <code className="text-foreground">signal</code>), so it
                  compiles and runs on Linux and macOS. No Windows support —
                  there is no POSIX process model to build on there.
                </p>
              </div>
            </div>
          </div>
        </section>
      </main>

      <footer className="border-t border-border px-6 py-10">
        <div className="mx-auto flex w-full max-w-5xl flex-col items-start justify-between gap-3 font-mono text-xs text-muted sm:flex-row sm:items-center">
          <span>ShellX — a Unix shell built from scratch in C.</span>
          <span>Licensed under GPLv3.</span>
        </div>
      </footer>
    </div>
  );
}
