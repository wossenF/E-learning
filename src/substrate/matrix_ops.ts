// matrix_ops.ts - small linear algebra utilities (matrix multiply, transpose, basic ops)

export type Matrix = number[][];

export function zeros(r:number,c:number): Matrix { const m:Matrix=[]; for(let i=0;i<r;i++){ m.push(new Array(c).fill(0)); } return m; }

export function transpose(a:Matrix): Matrix { const r=a.length; const c = r? a[0].length:0; const out = zeros(c,r); for(let i=0;i<r;i++) for(let j=0;j<c;j++) out[j][i]=a[i][j]; return out; }

export function multiply(a:Matrix,b:Matrix): Matrix { const ar=a.length; const ac = ar? a[0].length:0; const br = b.length; const bc = br? b[0].length:0; if(ac!==br) throw new Error('shape'); const out=zeros(ar, bc); for(let i=0;i<ar;i++) for(let k=0;k<ac;k++){ const v=a[i][k]; for(let j=0;j<bc;j++) out[i][j]+= v * b[k][j]; } return out; }

export function dotProduct(a:number[], b:number[]): number { const n=Math.min(a.length,b.length); let s=0; for(let i=0;i<n;i++) s+=a[i]*b[i]; return s; }

export function scale(a:Matrix, s:number): Matrix { return a.map(row=> row.map(v=>v*s)); }

export function add(a:Matrix,b:Matrix): Matrix { const r=a.length; const c = r? a[0].length:0; const out=zeros(r,c); for(let i=0;i<r;i++) for(let j=0;j<c;j++) out[i][j]= (a[i][j]||0) + (b[i] && b[i][j] || 0); return out; }

export function identity(n:number): Matrix { const m=zeros(n,n); for(let i=0;i<n;i++) m[i][i]=1; return m; }

export function normL2(v:number[]): number { return Math.sqrt(v.reduce((s,x)=>s+x*x,0)); }
