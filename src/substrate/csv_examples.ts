// csv_examples.ts - examples and small utilities for CSV processing and ingestion
import { csvToObjects, parseCSVLine, objectsToCSV } from './extra_big17';
import { Row } from './extra_big16';

export function makeSampleCSV(n=10): string {
  const header = ['id','name','value'];
  const lines = [header.join(',')];
  for(let i=0;i<n;i++) lines.push([i, 'name'+i, (Math.random()*100).toFixed(2)].join(','));
  return lines.join('\n');
}

export function roundTripCSV(n=10): boolean {
  const csv = makeSampleCSV(n); const rows = csvToObjects(csv); const out = objectsToCSV(rows); const reparsed = csvToObjects(out); return reparsed.length === rows.length;
}

export function headerFromLine(line:string): string[] { return parseCSVLine(line); }

export function sampleImportPipeline(csvText:string): Row[] { const rows = csvToObjects(csvText); // simple transform
  return rows.map(r=>{ const o:any={}; for(const k of Object.keys(r)) o[k.trim()] = (r as any)[k]; return o; }); }
