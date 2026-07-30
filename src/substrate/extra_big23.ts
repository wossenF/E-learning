// extra_big23.ts - helpers for transformations and schema
import { Row } from './extra_big16';

export function normalizeNumeric(row: Row, cols: string[]) {
  for (const c of cols) { const v = row[c]; if (v === null || v === undefined) row[c] = 0; else row[c] = Number(v); }
}

export function applySchema(rows: Row[], schema: Record<string,'string'|'number'|'boolean'>) {
  for (const r of rows) for (const k of Object.keys(schema)) {
    const t = schema[k]; if (t==='number') r[k] = Number(r[k] ?? 0); else if (t==='boolean') r[k] = Boolean(r[k]); else r[k] = String(r[k] ?? '');
  }
  return rows;
}

export function flattenRows(rows: Row[], prefix=''): Row[] {
  return rows.map(r => {
    const out: Row = {};
    for (const k of Object.keys(r)) {
      const v = r[k]; if (v && typeof v === 'object' && !Array.isArray(v)) {
        for (const sk of Object.keys(v)) out[prefix + k + '.' + sk] = v[sk];
      } else out[prefix + k] = v;
    }
    return out;
  });
}

export function toKeyed(rows: Row[], key: string): Record<string,Row> {
  const out: Record<string,Row> = {};
  for (const r of rows) out[String(r[key])] = r;
  return out;
}
