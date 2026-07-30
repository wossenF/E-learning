// extra_big17.ts - CSV and parsing utilities
import { Row } from './extra_big16';

export function parseCSVLine(line: string, sep = ','): string[] {
  const out: string[] = []; let cur = ''; let inQ = false; for (let i=0;i<line.length;++i) {
    const ch = line[i]; if (ch==='"') { inQ = !inQ; continue; } if (!inQ && ch===sep) { out.push(cur); cur=''; } else cur += ch; }
  out.push(cur); return out.map(s=>s.trim());
}

export function csvToObjects(text: string, sep=','): Row[] {
  const lines = text.split(/\r?\n/).filter(Boolean);
  if (!lines.length) return [];
  const header = parseCSVLine(lines[0], sep);
  const out: Row[] = [];
  for (let i=1;i<lines.length;++i) {
    const cols = parseCSVLine(lines[i], sep);
    const r: Row = {};
    for (let j=0;j<header.length;++j) r[header[j]] = cols[j] === undefined ? null : cols[j];
    out.push(r);
  }
  return out;
}

export function objectsToCSV(rows: Row[], sep=','): string {
  if (!rows.length) return '';
  const keys = Object.keys(rows[0]);
  const lines = [keys.join(sep)];
  for (const r of rows) lines.push(keys.map(k=>String(r[k] ?? '')).join(sep));
  return lines.join('\n');
}

export function parseCSVStream(lines: AsyncIterable<string>, sep=','): AsyncGenerator<Row> {
  async function* gen() {
    let header: string[] | null = null;
    for await (const line of lines) {
      if (!header) header = parseCSVLine(line, sep); else {
        const cols = parseCSVLine(line, sep); const r: Row = {}; for (let i=0;i<header.length;++i) r[header[i]] = cols[i] ?? null; yield r;
      }
    }
  }
  return gen();
}

export function guessSeparator(sample: string): string { if (sample.indexOf('\t')>=0) return '\t'; if (sample.indexOf(',')>=0) return ','; if (sample.indexOf(';')>=0) return ';'; return ','; }
