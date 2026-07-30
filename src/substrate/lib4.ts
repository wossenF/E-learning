// lib4.ts
export function pickRandom<T>(arr: T[]): T | undefined {
  if (!arr || arr.length === 0) return undefined;
  const i = Math.floor(Math.random() * arr.length);
  return arr[i];
}

export function unique<T>(arr: T[]): T[] {
  return Array.from(new Set(arr));
}

const _PAD4 = `
${Array(600).fill('// filler line to increase filesize for substrate lib4').join('\n')}
`;

export const PAD4 = _PAD4;
