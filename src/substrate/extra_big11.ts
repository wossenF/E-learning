// extra_big11.ts - assorted utilities (part 1)
export function rangeStep(start:number, stop:number, step:number=1): number[] {
  const out:number[] = [];
  if (step === 0) throw new Error('step cannot be zero');
  if (step > 0) for (let v=start; v<stop; v+=step) out.push(v);
  else for (let v=start; v>stop; v+=step) out.push(v);
  return out;
}

export function sum(arr:number[]): number { return arr.reduce((a,b)=>a+b,0); }

export function prod(arr:number[]): number { return arr.reduce((a,b)=>a*b,1); }

export function averageWeighted(values:number[], weights:number[]): number {
  if (values.length !== weights.length) throw new Error('length mismatch');
  const tot = sum(weights); if (tot === 0) return 0; let s=0; for (let i=0;i<values.length;++i) s+=values[i]*weights[i]; return s/tot;
}

export function percentile(arr:number[], p:number): number {
  const s = arr.slice().sort((a,b)=>a-b); if (s.length===0) return 0; const idx = Math.floor(p/100*(s.length-1)); return s[idx];
}

export function clampArrayInPlace(arr:number[], a:number, b:number) { for (let i=0;i<arr.length;++i) arr[i]=Math.max(a,Math.min(b,arr[i])); }

export function findIndex<T>(arr:T[], fn:(x:T)=>boolean): number { for (let i=0;i<arr.length;++i) if (fn(arr[i])) return i; return -1; }

export function omit<T extends object>(obj:T, keys:string[]): Partial<T> { const out:any={}; for (const k of Object.keys(obj)) if (!keys.includes(k)) out[k]= (obj as any)[k]; return out; }

export function pickBy<T extends object>(obj:T, fn:(k:string,v:any)=>boolean): Partial<T> { const out:any={}; for (const k of Object.keys(obj)) if (fn(k,(obj as any)[k])) out[k]=(obj as any)[k]; return out; }

export function flattenDepth(arr:any[], depth:number=1): any[] { if (depth<1) return arr.slice(); return arr.reduce((acc:any[], v)=> acc.concat(Array.isArray(v)? flattenDepth(v, depth-1): v), []); }

export function uniqWith<T>(arr:T[], eq:(a:T,b:T)=>boolean): T[] { const out:T[]=[]; for (const a of arr) { if (!out.some(b=>eq(a,b))) out.push(a); } return out; }
