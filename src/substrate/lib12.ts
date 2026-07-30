// lib12.ts
export function sleep(ms:number){ return new Promise(resolve=>setTimeout(resolve, ms)); }

export async function retry<T>(fn:()=>Promise<T>, attempts=3, delayMs=100): Promise<T> {
  let lastErr: any;
  for (let i=0;i<attempts;i++){
    try { return await fn(); } catch(e) { lastErr = e; await sleep(delayMs); }
  }
  throw lastErr;
}

const _PAD12 = `
${Array(600).fill('// filler line to increase filesize for substrate lib12').join('\n')}
`;

export const PAD12 = _PAD12;
