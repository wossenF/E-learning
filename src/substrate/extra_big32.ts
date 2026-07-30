// extra_big32.ts - deep utilities for objects
export function deepClone<T>(obj:T): T { return JSON.parse(JSON.stringify(obj)); }

export function deepMerge(a:any,b:any): any {
  const out = Array.isArray(a)? a.slice() : Object.assign({}, a);
  for (const k of Object.keys(b)) {
    if (b[k] && typeof b[k] === 'object' && !Array.isArray(b[k]) && out[k] && typeof out[k] === 'object') out[k] = deepMerge(out[k], b[k]); else out[k] = b[k];
  }
  return out;
}

export function pick(obj:any, keys:string[]) { const out:any={}; for (const k of keys) if (k in obj) out[k]=obj[k]; return out; }
