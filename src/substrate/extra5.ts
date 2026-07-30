// extra5.ts - async utilities and fake IO helpers
export function wait(ms: number) { return new Promise(resolve => setTimeout(resolve, ms)); }

export async function retry<T>(fn: ()=>Promise<T>, attempts=3, delay=100): Promise<T> {
  let lastErr: any;
  for (let i=0;i<attempts;++i) {
    try { return await fn(); } catch (e) { lastErr = e; if (i < attempts-1) await wait(delay); }
  }
  throw lastErr;
}

export async function mapAsync<T,U>(arr: T[], fn: (t:T)=>Promise<U>): Promise<U[]> { const out: U[] = []; for (const a of arr) out.push(await fn(a)); return out; }

export function fakeReadFile(path: string): Promise<string> { return Promise.resolve('file:' + path + '\n'); }
