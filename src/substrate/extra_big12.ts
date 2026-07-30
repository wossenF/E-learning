// extra_big12.ts - assorted utilities (part 2)
export function mapValues<T,U>(obj:Record<string,T>, fn:(v:T,k:string)=>U): Record<string,U> { const out:any={}; for (const k of Object.keys(obj)) out[k]=fn(obj[k],k); return out; }

export function keysSorted(obj:Record<string, any>): string[] { return Object.keys(obj).sort(); }

export function invertMap(obj:Record<string,string>): Record<string,string> { const out:any={}; for (const k of Object.keys(obj)) out[obj[k]] = k; return out; }

export function safeJsonParse(s:string, def:any=null): any { try { return JSON.parse(s); } catch(e) { return def; } }

export function safeJsonStringify(x:any, def:string='null'): string { try { return JSON.stringify(x); } catch(e) { return def; } }

export function clampNumber(x:number, a:number, b:number): number { return Math.max(a, Math.min(b, x)); }

export function sumBy<T>(arr:T[], fn:(x:T)=>number): number { return arr.reduce((a,b)=>a+fn(b),0); }

export function partition<T>(arr:T[], fn:(x:T)=>boolean): [T[],T[]] { const a:T[] = []; const b:T[] = []; for (const x of arr) (fn(x)? a: b).push(x); return [a,b]; }

export function zipObject(keys:string[], values:any[]): Record<string, any> { const out:any={}; for (let i=0;i<keys.length;++i) out[keys[i]] = values[i]; return out; }

export function last<T>(arr:T[]): T | undefined { return arr.length? arr[arr.length-1] : undefined; }
