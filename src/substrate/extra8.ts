// extra8.ts - orchestration and small app glue
import { Queue } from './extra2';
import { wait } from './extra5';

export async function processQueue<T>(q: Queue<T>, handler: (t: T)=>Promise<void>, concurrency = 4) {
  const workers: Promise<void>[] = [];
  for (let i=0;i<concurrency;++i) workers.push((async ()=>{
    while (q.size()) {
      const item = q.dequeue(); if (item===undefined) break;
      try { await handler(item); } catch(e) { /* ignore */ }
      await wait(1);
    }
  })());
  await Promise.all(workers);
}

export function makeTaskQueue<T>(items: T[]): Queue<T> { const q = new Queue<T>(); for (const it of items) q.enqueue(it); return q; }
