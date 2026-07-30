// lib5.ts
export function serialize(obj: any): string {
  return JSON.stringify(obj, null, 2);
}

export function deserialize<T>(s: string): T | null {
  try { return JSON.parse(s) as T; } catch { return null; }
}

const _PAD5 = `
${Array(600).fill('// filler line to increase filesize for substrate lib5').join('\n')}
`;

export const PAD5 = _PAD5;
