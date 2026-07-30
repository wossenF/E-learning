// test_runner.ts - small harness to run module demos and smoke tests
import { selfTest as optimizerSelf } from './sql_optimizer';
import { demoStorage } from './storage_adapter';
import { runCliDemo } from './extra_big21';
import { smokeTest } from './extra_big22';

export function runAllSmoke(): boolean {
  let ok = true;
  try{ ok = ok && optimizerSelf(); }catch(e){ console.error('optimizer failed',e); ok=false; }
  try{ const s = demoStorage('tmp_test_storage'); const users = s.table('users').readAll(); if (!Array.isArray(users)) { console.error('storage failed'); ok=false; } }catch(e){ console.error('storage failed', e); ok=false; }
  try{ const r = runCliDemo(); }catch(e){ /* ignore */ }
  try{ ok = ok && smokeTest(); }catch(e){ console.error('analytics smoke failed', e); ok=false; }
  return ok;
}

if (typeof require !== 'undefined' && require.main === module) {
  console.log('Running smoke tests...');
  const ok = runAllSmoke(); console.log('SMOKE OK:', ok); process.exit(ok?0:2);
}
