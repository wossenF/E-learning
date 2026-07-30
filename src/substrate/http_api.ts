// http_api.ts - tiny HTTP API wrappers (Express-style minimal)
import { Row } from './extra_big16';
import * as http from 'http';

export type Handler = (req:http.IncomingMessage, res:http.ServerResponse, params?:Record<string,string>)=>Promise<void> | void;

export class TinyRouter {
  private routes: { method:string, path: RegExp, keys:string[], handler: Handler }[] = [];
  add(method:string, path:string, handler:Handler){ const { re, keys } = pathToRegex(path); this.routes.push({ method: method.toUpperCase(), path: re, keys, handler }); }
  handle(req:http.IncomingMessage, res:http.ServerResponse){ const url = req.url || '/'; const method = (req.method||'GET').toUpperCase(); for(const r of this.routes){ const m = r.path.exec(url); if (!m || r.method !== method) continue; const params:any={}; for(let i=1;i<m.length;i++) params[r.keys[i-1]] = m[i]; try{ const maybe = r.handler(req,res,params); if (maybe && typeof (maybe as any).then === 'function') (maybe as Promise<any>).catch(err=>{ res.statusCode=500; res.end(String(err)); }); }catch(e:any){ res.statusCode=500; res.end(String(e)); } return; } res.statusCode=404; res.end('not found'); }
}

function pathToRegex(path:string){ const parts = path.split('/').filter(Boolean); const keys:string[]=[]; const reParts = parts.map(p=>{ if (p.startsWith(':')){ keys.push(p.slice(1)); return '([^/]+)'; } return p.replace(/[.*+?^${}()|[\]\\]/g,'\\$&'); }); const re = new RegExp('^/' + reParts.join('/') + '(?:\?.*)?$'); return { re, keys };
}

export function createServer(router:TinyRouter, port=0){ const srv = http.createServer((req,res)=>router.handle(req,res)); srv.listen(port); return srv; }

export async function fetchJson(url:string, opts:any={}){ return new Promise<any>((res,rej)=>{ const req = http.get(url,(r)=>{ let s=''; r.on('data',c=>s+=c); r.on('end',()=>{ try{ res(JSON.parse(s)); }catch(e){ rej(e); } }); }); req.on('error', rej); if (opts.timeout) req.setTimeout(opts.timeout, ()=>req.abort()); }); }
