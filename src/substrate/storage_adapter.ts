// storage_adapter.ts - small filesystem-backed storage adapter (JSON lines)
import { Row } from './extra_big16';
import * as fs from 'fs';
import * as path from 'path';

export class JSONLTable {
  constructor(public filePath:string){
    const dir = path.dirname(filePath); if (!fs.existsSync(dir)) fs.mkdirSync(dir,{recursive:true});
    if (!fs.existsSync(filePath)) fs.writeFileSync(filePath,'','utf8');
  }
  append(row:Row){ const line = JSON.stringify(row); fs.appendFileSync(this.filePath, line+'\n', 'utf8'); }
  readAll(): Row[] { const txt = fs.readFileSync(this.filePath,'utf8'); if (!txt) return []; return txt.split(/\r?\n/).filter(Boolean).map(l=>{ try{return JSON.parse(l);}catch(e){return {};}}); }
  truncate(){ fs.writeFileSync(this.filePath,'','utf8'); }
}

export class StorageEngine {
  private tables = new Map<string, JSONLTable>();
  constructor(public baseDir:string){ if (!fs.existsSync(baseDir)) fs.mkdirSync(baseDir,{recursive:true}); }
  table(name:string): JSONLTable { if (!this.tables.has(name)) this.tables.set(name, new JSONLTable(path.join(this.baseDir, name + '.jsonl'))); return this.tables.get(name)!; }
  importCSV(name:string, csvText:string, sep=','){
    const lines = csvText.split(/\r?\n/).filter(Boolean); if (!lines.length) return; const header = lines[0].split(sep).map(s=>s.trim()); for(let i=1;i<lines.length;i++){ const cols = lines[i].split(sep); const row:any={}; for(let j=0;j<header.length;j++) row[header[j]] = cols[j] === undefined ? null : cols[j]; this.table(name).append(row); }
  }
  exportJSON(name:string): string { return JSON.stringify(this.table(name).readAll(), null, 2); }
}

// demo/test helper
export function demoStorage(dir='tmp_storage'){
  const s = new StorageEngine(dir);
  s.table('users').truncate();
  s.table('users').append({id:1,name:'alice',age:30});
  s.table('users').append({id:2,name:'bob',age:25});
  return s;
}
