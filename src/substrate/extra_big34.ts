// extra_big34.ts - CLI arg parsing helpers
export function parseArgs(argv:string[]): Record<string,string|boolean> {
  const out:any={}; let i=0; while (i<argv.length) {
    const a = argv[i++]; if (!a) break; if (a.startsWith('--')) { const k=a.slice(2); const v = argv[i] && !argv[i].startsWith('-') ? argv[i++] : true; out[k]=v; } else if (a.startsWith('-')) { const k=a.slice(1); out[k]=true; } else { out._ = out._ || []; out._.push(a); }
  } return out;
}

export function usage(prog:string, opts: string[]) { return `usage: ${prog} [options]\nOptions:\n${opts.join('\n')}`; }
