// extra_big10.ts - high-level orchestration helpers
import { Logger } from './extra_big5';
export function runPipeline(steps: (()=>Promise<void>)[], logger?: Logger) {
  return steps.reduce((p, step, idx) => p.then(()=> { logger && logger.info(`step ${idx}`); return step(); }), Promise.resolve());
}

export async function sleep(ms:number) { return new Promise(r=>setTimeout(r,ms)); }

export function identityAsync<T>(x:T): Promise<T> { return Promise.resolve(x); }
