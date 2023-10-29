type Line =
  | { kind: "cmd"; prompt: string; text: string; comment?: string }
  | { kind: "out"; text: string };

const SESSION: Line[] = [
  { kind: "cmd", prompt: "$", text: "./shellx" },
  { kind: "cmd", prompt: "shellx>", text: "pwd" },
  { kind: "out", text: "/home/akshat" },
  { kind: "cmd", prompt: "shellx>", text: "ls" },
  { kind: "out", text: "projects  notes  Downloads" },
  {
    kind: "cmd",
    prompt: "shellx>",
    text: "#projects",
    comment: "fuzzy history search",
  },
  {
    kind: "cmd",
    prompt: "shellx>",
    text: "cd projects",
    comment: "recalled from history",
  },
  { kind: "cmd", prompt: "shellx>", text: "exit" },
];

export default function Terminal() {
  return (
    <div className="w-full max-w-2xl overflow-hidden rounded-lg border border-border bg-surface shadow-[0_0_0_1px_rgba(255,255,255,0.02),0_20px_60px_-20px_rgba(0,0,0,0.6)]">
      <div className="flex items-center gap-2 border-b border-border bg-surface-2 px-4 py-3">
        <span className="h-3 w-3 rounded-full bg-[#ff5f56]" />
        <span className="h-3 w-3 rounded-full bg-[#ffbd2e]" />
        <span className="h-3 w-3 rounded-full bg-[#27c93f]" />
        <span className="ml-3 font-mono text-xs text-muted">
          akshat@localhost — shellx
        </span>
      </div>
      <div className="px-5 py-5 font-mono text-[13px] leading-6 sm:text-sm">
        {SESSION.map((line, i) => (
          <div key={i} className="whitespace-pre-wrap break-words">
            {line.kind === "cmd" ? (
              <>
                <span className="text-accent">{line.prompt}</span>{" "}
                <span className="text-foreground">{line.text}</span>
                {line.comment ? (
                  <span className="text-muted">
                    {"  # "}
                    {line.comment}
                  </span>
                ) : null}
              </>
            ) : (
              <span className="text-muted">{line.text}</span>
            )}
          </div>
        ))}
        <div>
          <span className="text-accent">shellx&gt;</span>{" "}
          <span className="caret inline-block h-4 w-2 -translate-y-0.5 bg-accent align-middle" />
        </div>
      </div>
    </div>
  );
}
