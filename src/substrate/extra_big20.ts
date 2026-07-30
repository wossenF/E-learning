// extra_big20.ts - visualization stubs (text-based)
import { Row } from './extra_big16';

export function asciiTable(rows: Row[], cols?: string[]): string {
  if (!rows.length) return '';
  const keys = cols ?? Object.keys(rows[0]);
  const widths = keys.map(k => Math.max(k.length, ...rows.map(r=>String(r[k]??'').length)));
  const sep = widths.map(w=>'-'.repeat(w+2)).join('+');
  const header = keys.map((k,i)=> ' ' + k.padEnd(widths[i]) + ' ').join('|');
  const lines = [sep, header, sep];
  for (const r of rows) lines.push(keys.map((k,i)=> ' ' + String(r[k]??'').padEnd(widths[i]) + ' ').join('|'));
  lines.push(sep); return lines.join('\n');
}

export function sparkline(values: number[], width=40): string {
  if (!values.length) return '';
  const min = Math.min(...values); const max = Math.max(...values); const rng = Math.max(1, max-min);
  const step = Math.max(1, Math.floor(values.length/width)); const bars = '▁▂▃▄▅▆▇█';
  const out: string[] = [];
  for (let i=0;i<values.length;i+=step) {
    const v = values[i]; const idx = Math.floor((v-min)/rng*(bars.length-1)); out.push(bars[idx]);
  }
  return out.join('');
}

export function barChart(rows: Row[], labelCol:string, valueCol:string, maxWidth=50): string {
  const total = Math.max(...rows.map(r=>Number(r[valueCol]||0))); if (!rows.length) return '';
  const lines: string[] = [];
  for (const r of rows) {
    const label = String(r[labelCol]||''); const val = Number(r[valueCol]||0); const len = Math.round((val/total)*maxWidth);
    lines.push(label.padEnd(20) + '|' + '#'.repeat(len));
  }
  return lines.join('\n');
}
