// extra_big18.ts - windowing and aggregation utilities
import { Row } from './extra_big16';

export type WindowSpec = { size: number, slide?: number };

export function slidingWindow(rows: Row[], key: string, spec: WindowSpec): Row[][] {
  const groups = new Map<any, Row[]>();
  for (const r of rows) {
    const k = r[key]; if (!groups.has(k)) groups.set(k, []); groups.get(k)!.push(r);
  }
  const out: Row[][] = [];
  for (const [,g] of groups) {
    const s = spec.slide ?? spec.size; for (let i=0;i+spec.size<=g.length;i+=s) out.push(g.slice(i,i+spec.size));
  }
  return out;
}

export function windowAggregate(rows: Row[], key:string, spec:WindowSpec, agg:(rows:Row[])=>Row): Row[] {
  const windows = slidingWindow(rows,key,spec); return windows.map(w=>agg(w));
}

export function movingAverage(rows: Row[], col:string, window:number): number[] {
  const out:number[] = []; for (let i=0;i<rows.length;++i) { const start=Math.max(0,i-window+1); let s=0; for (let j=start;j<=i;++j) s+= Number(rows[j][col]||0); out.push(s/(i-start+1)); } return out;
}

export function cumulativeSum(rows: Row[], col:string): number[] { const out:number[]=[]; let s=0; for (const r of rows) { s+=Number(r[col]||0); out.push(s);} return out; }

export function topN(rows: Row[], col:string, n=10): Row[] { return rows.slice().sort((a,b)=>Number(b[col]||0)-Number(a[col]||0)).slice(0,n); }
