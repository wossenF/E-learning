// lib3.ts
export interface Item { id: string; data: Record<string, any>; }

export function makeItem(id: string, data: Record<string, any>): Item {
  return { id, data };
}

export function clone<T>(obj: T): T {
  return JSON.parse(JSON.stringify(obj));
}

const _PAD3 = `
${Array(600).fill('// filler line to increase filesize for substrate lib3').join('\n')}
`;

export const PAD3 = _PAD3;
