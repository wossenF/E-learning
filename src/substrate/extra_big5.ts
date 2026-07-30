// extra_big5.ts - logging and diagnostics
export enum LogLevel { DEBUG=10, INFO=20, WARN=30, ERROR=40 }

export class Logger {
  constructor(public name:string, public level:LogLevel=LogLevel.INFO){}
  log(l:LogLevel, msg:string){ if (l>=this.level) console.log(`[${this.name}] ${LogLevel[l]}: ${msg}`); }
  debug(m:string){ this.log(LogLevel.DEBUG,m); }
  info(m:string){ this.log(LogLevel.INFO,m); }
  warn(m:string){ this.log(LogLevel.WARN,m); }
  error(m:string){ this.log(LogLevel.ERROR,m); }
}

export function captureStack(): string { try { throw new Error('stack'); } catch(e:any) { return e.stack || ''; } }

export function measure<T>(fn: ()=>T): {result:T, ms:number} { const t0=Date.now(); const r=fn(); return { result: r, ms: Date.now()-t0 }; }
