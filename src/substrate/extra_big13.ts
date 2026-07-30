// extra_big13.ts - more algorithms
export function binarySearch<T>(arr:T[], cmp:(a:T)=>number): number {
  let lo=0, hi=arr.length-1; while (lo<=hi) { const mid = Math.floor((lo+hi)/2); const c = cmp(arr[mid]); if (c===0) return mid; if (c<0) lo = mid+1; else hi = mid-1; } return -1;
}

export function lowerBound<T>(arr:T[], key:any, cmp:(a:T,b:any)=>number): number { let lo=0, hi=arr.length; while (lo<hi) { const mid = Math.floor((lo+hi)/2); if (cmp(arr[mid], key) < 0) lo = mid+1; else hi = mid; } return lo; }

export function upperBound<T>(arr:T[], key:any, cmp:(a:T,b:any)=>number): number { let lo=0, hi=arr.length; while (lo<hi) { const mid = Math.floor((lo+hi)/2); if (cmp(arr[mid], key) <= 0) lo = mid+1; else hi = mid; } return lo; }

export function stablePartition<T>(arr:T[], fn:(x:T)=>boolean): T[] { const a:T[] = []; const b:T[] = []; for (const x of arr) (fn(x) ? a : b).push(x); return a.concat(b); }

export function rotate<T>(arr:T[], k:number): T[] { k%=arr.length; if (k<0) k+=arr.length; return arr.slice(k).concat(arr.slice(0,k)); }

export function combinations<T>(arr:T[], k:number): T[][] {
  const out:T[][]=[]; const n=arr.length; const comb = (start:number, path:T[]) => { if (path.length===k) { out.push(path.slice()); return; } for (let i=start;i<n;++i) { path.push(arr[i]); comb(i+1,path); path.pop(); } };
  comb(0,[]); return out;
}
