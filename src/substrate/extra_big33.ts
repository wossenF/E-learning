// extra_big33.ts - validation helpers
import { Row } from './extra_big16';

export function isEmail(s:string): boolean { return /^[^@\s]+@[^@\s]+\.[^@\s]+$/.test(s); }
export function isUUID(s:string): boolean { return /^[0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$/i.test(s); }

export function validateRows(rows: Row[], rules: Record<string,(v:any)=>boolean>): string[] {
  const out: string[] = [];
  for (let i=0;i<rows.length;++i) {
    const r = rows[i]; for (const k of Object.keys(rules)) { if (!rules[k](r[k])) out.push(`row ${i} failed ${k}`); }
  }
  return out;
}
