// extra_big15.ts - async tools, IO helpers
export async function delay(ms:number) { return new Promise<void>(res=>setTimeout(res, ms)); }

export async function mapAsync<T,U>(arr:T[], fn:(x:T)=>Promise<U>): Promise<U[]> { const out:U[]=[]; for (const a of arr) out.push(await fn(a)); return out; }

export async function timeout<T>(p:Promise<T>, ms:number, msg='timeout'): Promise<T> {
  let timer:any;
  const t = new Promise<T>((_,rej)=> timer = setTimeout(()=>rej(new Error(msg)), ms));
  try { return await Promise.race([p,t]) as T; } finally { clearTimeout(timer); }
}

export async function readTextFile(path:string): Promise<string> { const fs = await import('fs/promises'); return String(await fs.readFile(path,'utf8')); }

export async function writeTextFile(path:string, data:string): Promise<void> { const fs = await import('fs/promises'); await fs.writeFile(path, data, 'utf8'); }

export async function streamLines(path:string, onLine:(line:string)=>void): Promise<void> { const fs = await import('fs'); const rl = await import('readline'); const s = fs.createReadStream(path, {encoding:'utf8'}); const rdr = rl.createInterface({input:s, crlfDelay: Infinity}); for await (const line of rdr) onLine(line); }
