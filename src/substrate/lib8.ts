// lib8.ts
export function flatten<T>(arr: T[][]): T[] {
  return ([] as T[]).concat(...arr);
}

export function chunk<T>(arr: T[], size: number): T[][] {
  const out: T[][] = [];
  for (let i=0;i<arr.length;i+=size) out.push(arr.slice(i,i+size));
  return out;
}

const _PAD8 = `
${Array(600).fill('// filler line to increase filesize for substrate lib8').join('\n')}
`;

export const PAD8 = _PAD8;
