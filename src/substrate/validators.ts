// validators.ts - JSON schema lite and validation helpers
export type Schema = { type?: 'object'|'array'|'string'|'number'|'boolean'|'any', properties?: Record<string,Schema>, required?: string[], items?: Schema };

export function validate(schema:Schema, data:any): {ok:boolean, errors:string[]} {
  const errors:string[] = [];
  function visit(s:Schema, d:any, path:string){ if(!s) return; if(s.type && s.type!=='any'){
    if(s.type==='string' && typeof d !== 'string') errors.push(`${path} expected string`);
    if(s.type==='number' && typeof d !== 'number') errors.push(`${path} expected number`);
    if(s.type==='boolean' && typeof d !== 'boolean') errors.push(`${path} expected boolean`);
    if(s.type==='object' && (typeof d !== 'object' || d === null || Array.isArray(d))) errors.push(`${path} expected object`);
    if(s.type==='array' && !Array.isArray(d)) errors.push(`${path} expected array`);
  }
  if(s.type==='object' && s.properties){ for(const k of Object.keys(s.properties)){ if(d && d[k] !== undefined) visit(s.properties[k], d[k], path? path+'.'+k : k); else if(s.required && s.required.includes(k)) errors.push(`${path?path+'.':''}${k} required`); } }
  if(s.type==='array' && s.items && Array.isArray(d)){ for(let i=0;i<d.length;i++) visit(s.items, d[i], path + '['+i+']'); }
  }
  visit(schema, data, ''); return { ok: errors.length===0, errors };
}

export function isEmail(s:string){ return /^[^@\s]+@[^@\s]+\.[^@\s]+$/.test(s); }
export function isPhone(s:string){ return /^\+?[0-9\s\-()]{7,30}$/.test(s); }

export function sanitizeString(s:any){ if(s==null) return ''; return String(s).trim(); }
