// extra_big24.ts - orchestration helpers and connectors (stubs)
import { Row } from './extra_big16';
import { readTextFile, writeTextFile } from './extra_big15';

export async function loadCsvFile(path:string): Promise<Row[]> {
  const txt = await readTextFile(path); const sep = txt.indexOf('\t')>=0? '\t' : ','; return (await import('./extra_big17')).csvToObjects(txt, sep);
}

export async function saveJson(path:string, rows:Row[]) { await writeTextFile(path, JSON.stringify(rows, null, 2)); }

export async function pipelineTransform(inputPath:string, outPath:string, schema:Record<string,'string'|'number'|'boolean'>) {
  const rows = await loadCsvFile(inputPath); applySchema(rows, schema); await saveJson(outPath, rows);
}

export function applySchema(rows: Row[], schema: Record<string,'string'|'number'|'boolean'>) {
  for (const r of rows) for (const k of Object.keys(schema)) {
    const t = schema[k]; if (t==='number') r[k] = Number(r[k] ?? 0); else if (t==='boolean') r[k] = Boolean(r[k]); else r[k] = String(r[k] ?? '');
  }
}

export async function connStubFetch(url:string): Promise<{status:number, body:string}> { return { status:200, body: 'ok' }; }
