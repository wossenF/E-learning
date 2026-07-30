// extra_big36.ts - compatibility and polyfills
export function ensureArray<T>(x:T|T[]|null|undefined): T[] { if (x==null) return []; return Array.isArray(x)? x : [x]; }

export function ensureString(x:any): string { if (x==null) return ''; if (typeof x==='string') return x; return String(x); }

export function noop() {}

export function once<F extends (...args:any[])=>any>(fn:F): F { let called=false; let res:any; return ((...args:any[])=>{ if (!called) { called=true; res = fn(...args); } return res; }) as any as F; }
