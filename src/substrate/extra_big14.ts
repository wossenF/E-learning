// extra_big14.ts - data validation and transformation
export function mapEntries(obj:any, fn:(k:string,v:any)=>[string,any]) { const out:any={}; for (const k of Object.keys(obj)) { const [nk,nv] = fn(k,obj[k]); out[nk]=nv; } return out; }

export function pickDefined<T extends object>(obj:T): Partial<T> { const out:any={}; for (const k of Object.keys(obj)) { const v = (obj as any)[k]; if (v !== undefined) out[k]=v; } return out; }

export function deepFreeze<T>(o:T): T { Object.freeze(o); for (const k of Object.keys(o as any)) { const v = (o as any)[k]; if (typeof v === 'object' && v !== null && !Object.isFrozen(v)) deepFreeze(v); } return o; }

export function toNumberArray(arr:any[]): number[] { return arr.map(x=>Number(x)); }

export function safeGet(obj:any, path:string[], def:any=undefined) { let cur = obj; for (const p of path) { if (!cur) return def; cur = cur[p]; } return cur===undefined ? def : cur; }
