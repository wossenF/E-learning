// extra_big27.ts - math and statistics helpers
export function mean(arr:number[]): number { if (!arr.length) return 0; return arr.reduce((a,b)=>a+b,0)/arr.length; }
export function variance(arr:number[], sample=true): number { if (arr.length<2) return 0; const m = mean(arr); const s = arr.reduce((a,b)=>a+(b-m)*(b-m),0); return s / (arr.length - (sample?1:0)); }
export function stddev(arr:number[], sample=true): number { return Math.sqrt(variance(arr,sample)); }

export function linspace(a:number,b:number,n:number): number[] { if (n<=0) return []; if (n===1) return [a]; const out:number[]=[]; const step=(b-a)/(n-1); for (let i=0;i<n;++i) out.push(a+step*i); return out; }

export function dot(a:number[], b:number[]): number { let s=0; for (let i=0;i<a.length && i<b.length;++i) s+=a[i]*b[i]; return s; }

export function correlate(a:number[], b:number[]): number { const ma=mean(a), mb=mean(b); const na=a.length; let num=0, da=0, db=0; for (let i=0;i<na && i<b.length;++i) { const ra=a[i]-ma, rb=b[i]-mb; num+=ra*rb; da+=ra*ra; db+=rb*rb; } return num/Math.sqrt(Math.max(1e-12, da*db)); }
