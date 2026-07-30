// lib2.ts
export class Counter {
  private n: number;
  constructor(start = 0) { this.n = start; }
  inc() { this.n += 1; return this.n; }
  dec() { this.n -= 1; return this.n; }
  value() { return this.n; }
}

export function range(start: number, end: number): number[] {
  const out: number[] = [];
  for (let i = start; i < end; i++) out.push(i);
  return out;
}

const _PAD2 = `
${Array(600).fill('// filler line to increase filesize for substrate lib2').join('\n')}
`;

export const PAD2 = _PAD2;
