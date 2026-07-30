// extra1.ts - utility collection
export function capitalize(s: string): string {
  if (!s) return s;
  return s.charAt(0).toUpperCase() + s.slice(1);
}

export function kebabToCamel(s: string): string {
  return s.split('-').map((part, i) => i ? capitalize(part) : part).join('');
}

export function deepClone<T>(obj: T): T {
  return JSON.parse(JSON.stringify(obj));
}

export function pick<T, K extends keyof T>(obj: T, keys: K[]): Pick<T, K> {
  const out = {} as Pick<T, K>;
  for (const k of keys) {
    if (k in obj) (out as any)[k] = obj[k];
  }
  return out;
}

export class SimpleMap<V> {
  private map: Record<string, V> = {};
  set(k: string, v: V) { this.map[k]=v; }
  get(k: string): V | undefined { return this.map[k]; }
  keys(): string[] { return Object.keys(this.map); }
}

export function uniq<T>(arr: T[]): T[] { return Array.from(new Set(arr)); }

// a few more helpers to enlarge the module
export function range(start: number, end?: number): number[] {
  if (end === undefined) { end = start; start = 0; }
  const out: number[] = [];
  for (let i = start; i < end; ++i) out.push(i);
  return out;
}

export function groupBy<T>(arr: T[], keyFn: (x: T) => string): Record<string, T[]> {
  const out: Record<string, T[]> = {};
  for (const x of arr) { const k = keyFn(x); (out[k] = out[k] || []).push(x); }
  return out;
}

export function debounce<F extends (...a:any[])=>void>(fn: F, ms: number) {
  let t: any = null;
  return (...args: any[]) => {
    if (t) clearTimeout(t);
    t = setTimeout(() => fn(...args), ms);
  };
}

export function throttle<F extends (...a:any[])=>void>(fn: F, ms: number) {
  let last = 0;
  return (...args: any[]) => {
    const now = Date.now(); if (now - last >= ms) { last = now; fn(...args); }
  };
}
