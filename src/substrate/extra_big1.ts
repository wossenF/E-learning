// extra_big1.ts - larger utility module (generated helpers)
export function padLeft(s:string, n:number, ch=' '): string { if (s.length>=n) return s; return ch.repeat(n-s.length)+s; }
export function padRight(s:string, n:number, ch=' '): string { if (s.length>=n) return s; return s+ch.repeat(n-s.length); }

export function randInt(min:number, max:number): number { return Math.floor(Math.random()*(max-min))+min; }

export function clampArray(arr:number[], a:number, b:number): number[] { return arr.map(x=>Math.max(a, Math.min(b,x))); }

export function circlePoints(cx:number, cy:number, r:number, n:number): {x:number,y:number}[] {
  const out: {x:number,y:number}[] = [];
  for (let i=0;i<n;++i) { const theta = (i/n)*2*Math.PI; out.push({x:cx+r*Math.cos(theta), y:cy+r*Math.sin(theta)}); }
  return out;
}

export function approxEqual(a:number,b:number,eps=1e-6){return Math.abs(a-b)<=eps}

export class Counter {
  private map: Record<string, number>={}; inc(k:string, n=1){ this.map[k]=(this.map[k]||0)+n;} val(k:string){return this.map[k]||0} keys(){return Object.keys(this.map)}
}

export function permutations<T>(arr:T[]): T[][] {
  const out: T[][] = [];
  const visit = (a:T[], l:number) => { if (l===a.length) out.push(a.slice()); else { for (let i=l;i<a.length;++i){ [a[l],a[i]]=[a[i],a[l]]; visit(a,l+1); [a[l],a[i]]=[a[i],a[l]]; } } };
  visit(arr,0); return out;
}

export function cartesianProduct<T>(arrays: T[][]): T[][] {
  return arrays.reduce((a,b)=> a.flatMap(x=>b.map(y=>[...x,y])), [[]] as T[][]);
}

export function chunk<T>(arr:T[], n:number): T[][] { const out: T[][]=[]; for (let i=0;i<arr.length;i+=n) out.push(arr.slice(i,i+n)); return out; }

export function stableSort<T>(arr:T[], cmp:(a:T,b:T)=>number): T[] { return arr.slice().sort(cmp); }

// a bunch of small utilities to bulk up the module
export function identity<T>(x:T):T{return x}
export function noop():void{}
export function times(n:number, fn:(i:number)=>void){ for(let i=0;i<n;++i) fn(i); }
