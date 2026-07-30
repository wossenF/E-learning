// cli_tools.ts - richer CLI helpers, config loader, and interactive prompt utilities
import * as fs from 'fs';
import * as path from 'path';

export function loadJSON(pathname:string){ if(!fs.existsSync(pathname)) return null; try{ return JSON.parse(fs.readFileSync(pathname,'utf8')); }catch(e){ return null; } }

export function saveJSON(pathname:string, data:any){ const dir = path.dirname(pathname); if(!fs.existsSync(dir)) fs.mkdirSync(dir,{recursive:true}); fs.writeFileSync(pathname, JSON.stringify(data, null, 2), 'utf8'); }

export function parseArgs(argv:string[]): Record<string,string|boolean> { const out:any={}; let i=0; while(i<argv.length){ const a=argv[i++]; if(!a) break; if(a.startsWith('--')){ const k=a.slice(2); const v = (argv[i] && !argv[i].startsWith('-')) ? argv[i++] : true; out[k]=v; } else if(a.startsWith('-')){ const k=a.slice(1); out[k]=true; } else { out._ = out._ || []; out._.push(a); } } return out; }

export function findProjectRoot(start:string): string | null { let cur = path.resolve(start); while(true){ if(fs.existsSync(path.join(cur,'package.json')) || fs.existsSync(path.join(cur,'.git'))) return cur; const up = path.dirname(cur); if(up===cur) return null; cur = up; } }

export function envOr(key:string, def:string){ return process.env[key] || def; }

export async function prompt(question:string, defaultVal?:string): Promise<string> { return new Promise((res)=>{ process.stdout.write(question + (defaultVal? ` [${defaultVal}] ` : ' ')); process.stdin.resume(); process.stdin.once('data', d=>{ process.stdin.pause(); const s = String(d).trim(); res(s || defaultVal || ''); }); }); }

export function readStdin(): Promise<string> { return new Promise((res)=>{ let s=''; process.stdin.setEncoding('utf8'); process.stdin.on('data', chunk=>s+=chunk); process.stdin.on('end', ()=>res(s)); }); }
