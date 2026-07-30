// fs_utils.ts - streaming file utilities and simple cache
import * as fs from 'fs';
import * as readline from 'readline';

export async function streamLines(path:string, onLine:(line:string)=>void){ const rs = fs.createReadStream(path,{encoding:'utf8'}); const rl = readline.createInterface({ input: rs, crlfDelay: Infinity }); for await (const line of rl) onLine(line); }

export function readJsonIfExists(path:string){ if(!fs.existsSync(path)) return null; try{ return JSON.parse(fs.readFileSync(path,'utf8')); }catch(e){ return null; } }

export function atomicWrite(path:string, data:string){ const tmp = path + '.tmp'; fs.writeFileSync(tmp, data, 'utf8'); fs.renameSync(tmp, path); }

export class SimpleCache<T> {
  private map = new Map<string, {ts:number, value:T}>(); private ttl:number;
  constructor(ttl=60000){ this.ttl = ttl; }
  get(key:string): T | undefined { const v = this.map.get(key); if(!v) return undefined; if(Date.now()-v.ts > this.ttl){ this.map.delete(key); return undefined; } return v.value; }
  set(key:string, val:T){ this.map.set(key, { ts: Date.now(), value: val }); }
}
