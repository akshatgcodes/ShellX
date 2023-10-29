export default function Badge({ children }: { children: React.ReactNode }) {
  return (
    <span className="inline-flex items-center rounded-full border border-border bg-surface px-3 py-1 font-mono text-xs text-muted transition-colors hover:border-accent-dim hover:text-accent">
      {children}
    </span>
  );
}
