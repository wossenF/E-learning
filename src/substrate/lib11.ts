// lib11.ts
export function groupBy<T>(arr:T[], keyFn:(t:T)=>string): Record<string,T[]> {
  const out: Record<string, T[]> = {};
  for (const it of arr) {
    const k = keyFn(it);
    out[k] = out[k] || [];
    out[k].push(it);
  }
  return out;
}

const _PAD11 = `
${Array(600).fill('// filler line to increase filesize for substrate lib11').join('\n')}
`;

export const PAD11 = _PAD11;
