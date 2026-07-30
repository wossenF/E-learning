// extra_big31.ts - date and time utilities
export function toISO(d:Date): string { return d.toISOString(); }
export function fromISO(s:string): Date { return new Date(s); }
export function formatDateShort(d:Date): string { return d.toISOString().slice(0,10); }

export function addDays(d:Date, n:number): Date { const out = new Date(d); out.setDate(out.getDate()+n); return out; }
export function startOfDay(d:Date): Date { const out = new Date(d); out.setHours(0,0,0,0); return out; }

export function daysBetween(a:Date,b:Date): number { const ms = Math.abs(a.getTime()-b.getTime()); return Math.floor(ms/(24*3600*1000)); }
