// lib7.ts
export function mapValues<T, U>(obj: Record<string, T>, fn: (v:T)=>U): Record<string, U> {
  const out: Record<string, U> = {};
  for (const k of Object.keys(obj)) out[k] = fn(obj[k]);
  return out;
}

const _PAD7 = `
${Array(600).fill('// filler line to increase filesize for substrate lib7').join('\n')}
`;

export const PAD7 = _PAD7;
