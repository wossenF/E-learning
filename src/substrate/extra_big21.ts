// extra_big21.ts - CLI utilities and small runner
import { exampleData, DataFrame } from './extra_big16';
import { asciiTable, sparkline } from './extra_big20';

export function printSample(n=10) {
  const df = exampleData(n);
  console.log('Sample DataFrame:');
  console.log(asciiTable(df.getRows().slice(0,10)));
  console.log('Sparkline of values:');
  console.log(sparkline(df.getRows().map(r=>Number(r.value||0))));
}

export function runCliDemo() {
  const df = exampleData(80);
  console.log('Top values:');
  const top = df.orderBy('value', false).limit(10).getRows();
  console.log(asciiTable(top));
}

// support for simple programmatic invocation
if (typeof require !== 'undefined' && require.main === module) {
  const arg = process.argv[2] || 'demo';
  if (arg === 'demo') runCliDemo(); else if (arg === 'sample') printSample(Number(process.argv[3]||10));
}
