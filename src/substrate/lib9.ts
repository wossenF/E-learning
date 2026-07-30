// lib9.ts
export function clamp(v:number, a:number, b:number) { return Math.max(a, Math.min(b, v)); }

export function lerp(a:number,b:number,t:number) { return a + (b-a)*t; }

const _PAD9 = `
${Array(600).fill('// filler line to increase filesize for substrate lib9').join('\n')}
`;

export const PAD9 = _PAD9;
