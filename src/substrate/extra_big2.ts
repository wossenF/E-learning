// extra_big2.ts - collection utilities
export function mergeObjects<T extends object>(a:T,b:T): T { return Object.assign({}, a, b); }

export function deepMerge(a:any,b:any): any {
  if (Array.isArray(a) && Array.isArray(b)) return a.concat(b);
  if (typeof a==='object' && typeof b==='object') {
    const out:any = {}; for (const k of Object.keys(a)) out[k]=a[k]; for (const k of Object.keys(b)) out[k]=deepMerge(out[k], b[k]); return out;
  }
  return b===undefined? a : b;
}

export function uniqueBy<T>(arr:T[], fn:(x:T)=>string): T[] { const seen = new Set<string>(); const out: T[]=[]; for (const x of arr) { const k = fn(x); if (!seen.has(k)) { seen.add(k); out.push(x); } } return out; }

export function zip<T>(...arrays: T[][]): T[][] { const n = Math.max(...arrays.map(a=>a.length)); const out: T[][]=[]; for (let i=0;i<n;++i){ out.push(arrays.map(a=>a[i])); } return out; }

export function pairwise<T>(arr:T[]): [T,T][] { const out: [T,T][] = []; for (let i=0;i+1<arr.length;++i) out.push([arr[i], arr[i+1]]); return out; }

export function slidingWindow<T>(arr:T[], w:number): T[][] { const out:T[][]=[]; for (let i=0;i+w<=arr.length;++i) out.push(arr.slice(i,i+w)); return out; }

export function sample<T>(arr:T[], k:number): T[] { const res: T[]=[]; for (let i=0;i<k;++i) res.push(arr[Math.floor(Math.random()*arr.length)]); return res; }

export function clampIndex(i:number, n:number): number { if (i<0) return 0; if (i>=n) return n-1; return i; }

export function objectMap<T,U>(obj:Record<string,T>, fn:(k:string,v:T)=>U): Record<string,U> { const out:Record<string,U>={}; for (const k of Object.keys(obj)) out[k]=fn(k,obj[k]); return out; }

export function values<T>(obj:Record<string,T>): T[] { return Object.keys(obj).map(k=>obj[k]); }
