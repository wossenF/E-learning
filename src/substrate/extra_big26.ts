// extra_big26.ts - string utilities
export function slugify(s:string): string { return s.toLowerCase().replace(/[^a-z0-9]+/g,'-').replace(/^-|-$/g,''); }

export function titleCase(s:string): string { return s.split(/\s+/).map(w=>w.charAt(0).toUpperCase()+w.slice(1).toLowerCase()).join(' '); }

export function indent(text:string, n=2): string { const pad = ' '.repeat(n); return text.split(/\r?\n/).map(l=>pad+l).join('\n'); }

export function wrap(text:string, width=80): string { const words = text.split(/\s+/); let line=''; const out:string[]=[]; for (const w of words) { if (line.length + w.length +1 > width) { out.push(line); line = w; } else { line = line? line + ' ' + w : w; } } if (line) out.push(line); return out.join('\n'); }

export function truncate(s:string, n:number, ellipsis='...'): string { if (s.length<=n) return s; return s.slice(0, Math.max(0, n-ellipsis.length)) + ellipsis; }
