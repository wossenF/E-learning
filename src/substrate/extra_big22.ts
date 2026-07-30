// extra_big22.ts - tests and examples for analytics modules
import { exampleData, DataFrame } from './extra_big16';
import { parseCSVLine, csvToObjects, objectsToCSV } from './extra_big17';
import { movingAverage, cumulativeSum } from './extra_big18';

export function smokeTest() {
  const df = exampleData(30);
  if (df.count() !== 30) throw new Error('count mismatch');
  const s = df.select(['id','value']).limit(5).getRows(); if (s.length !== 5) throw new Error('limit mismatch');
  const ma = movingAverage(df.getRows(), 'value', 3); if (ma.length !== df.count()) throw new Error('ma mismatch');
  const cs = cumulativeSum(df.getRows(), 'value'); if (cs.length !== df.count()) throw new Error('cs mismatch');
  const csv = objectsToCSV(df.getRows().slice(0,3)); const parsed = csvToObjects(csv); if (parsed.length !== 3) throw new Error('csv roundtrip');
  return true;
}

export function benchIterations(iters=1000) {
  const df = exampleData(200);
  const start = Date.now();
  for (let i=0;i<iters;++i) df.orderBy('value', false).limit(50).getRows();
  const dt = Date.now()-start; return dt;
}

if (typeof require !== 'undefined' && require.main === module) {
  console.log('Running smoke tests...');
  console.log('smoke:', smokeTest());
  console.log('bench ms:', benchIterations(200));
}
