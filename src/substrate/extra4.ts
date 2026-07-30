// extra4.ts - number utilities and math helpers
export function clamp(x: number, a: number, b: number): number { return Math.max(a, Math.min(b, x)); }

export function linspace(start: number, end: number, n: number): number[] {
  const out: number[] = []; if (n<=1) return [start]; const step = (end-start)/(n-1);
  for (let i=0;i<n;++i) out.push(start + i*step); return out;
}

export function mean(arr: number[]): number { return arr.reduce((a,b)=>a+b,0)/arr.length; }

export function median(arr: number[]): number { const s = arr.slice().sort((a,b)=>a-b); const m = Math.floor(s.length/2); return s.length%2===1 ? s[m] : (s[m-1]+s[m])/2; }

export function variance(arr: number[]): number { const m = mean(arr); return mean(arr.map(x=>(x-m)*(x-m))); }
