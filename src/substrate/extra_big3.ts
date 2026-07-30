// extra_big3.ts - date/time utilities
export function nowIso(): string { return new Date().toISOString(); }

export function formatDate(d: Date): string { return d.toISOString().replace('T',' ').replace('Z',''); }

export function daysBetween(a: Date, b: Date): number { return Math.round(Math.abs((a.getTime()-b.getTime())/(1000*60*60*24))); }

export function addDays(d: Date, days: number): Date { const n = new Date(d); n.setDate(n.getDate()+days); return n; }

export function startOfDay(d: Date): Date { const n = new Date(d); n.setHours(0,0,0,0); return n; }

export function endOfDay(d: Date): Date { const n=new Date(d); n.setHours(23,59,59,999); return n; }

export function isoToDate(s: string): Date { return new Date(s); }

export function timezoneOffsetHours(): number { return -new Date().getTimezoneOffset()/60; }

export function humanAgo(d: Date): string {
  const s = Math.floor((Date.now() - d.getTime())/1000);
  if (s < 60) return s + 's';
  if (s < 3600) return Math.floor(s/60) + 'm';
  if (s < 86400) return Math.floor(s/3600) + 'h';
  return Math.floor(s/86400) + 'd';
}
