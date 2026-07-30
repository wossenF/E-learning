// persistence.ts - write-ahead log and snapshot utilities (toy)
import * as fs from 'fs';

export class WAL {
  constructor(public path:string){ if(!fs.existsSync(path)) fs.writeFileSync(path,'','utf8'); }
  append(obj:any){ fs.appendFileSync(this.path, JSON.stringify(obj)+'\n','utf8'); }
  readAll(){ return fs.readFileSync(this.path,'utf8').split(/\r?\n/).filter(Boolean).map(l=>{ try{return JSON.parse(l);}catch(e){return null;} }); }
  clear(){ fs.writeFileSync(this.path,'','utf8'); }
}

export function snapshot(path:string, state:any){ fs.writeFileSync(path, JSON.stringify(state,null,2),'utf8'); }
export function loadSnapshot(path:string){ if(!fs.existsSync(path)) return null; try{return JSON.parse(fs.readFileSync(path,'utf8')); }catch(e){ return null; } }

export function restoreFromWAL(wal:WAL, snapshotPath?:string){ const snap = snapshotPath? loadSnapshot(snapshotPath) : null; const state = snap || {}; for(const op of wal.readAll()){ if(!op) continue; // naive: support set key operations
    if(op.type==='set') state[op.key]=op.value; }
  return state;
}
