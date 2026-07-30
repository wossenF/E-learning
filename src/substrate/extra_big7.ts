// extra_big7.ts - validation heavy module
export function isUUID(s:string): boolean { return /^[0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$/i.test(s); }

export function assert(cond:boolean, msg:string){ if (!cond) throw new Error(msg); }

export function ensure<T>(x:T|null|undefined, msg:string): T { if (x===null || x===undefined) throw new Error(msg); return x; }

export function tryParseInt(s:string, def:number=0): number { const n = parseInt(s,10); return isNaN(n)?def:n; }
