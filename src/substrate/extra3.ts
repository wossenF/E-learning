// extra3.ts - string and text utilities
export function slugify(s: string): string {
  return s.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/(^-|-$)/g, '');
}

export function titleCase(s: string): string {
  return s.split(' ').map(w=>w.charAt(0).toUpperCase()+w.slice(1)).join(' ');
}

export function leftPad(s: string, len: number, ch=' '): string {
  if (s.length >= len) return s; return ch.repeat(len - s.length) + s;
}

export function repeat(s: string, n: number): string { return new Array(n+1).join(s); }

export function wrap(text: string, width = 80): string {
  const words = text.split(/\s+/); let line = ''; const out: string[] = [];
  for (const w of words) {
    if ((line + ' ' + w).trim().length > width) { out.push(line.trim()); line = w; } else { line = (line + ' ' + w).trim(); }
  }
  if (line) out.push(line);
  return out.join('\n');
}
