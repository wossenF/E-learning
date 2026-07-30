// lib6.ts
export function debounce<F extends (...args:any[])=>any>(fn: F, wait=50) {
  let t: any = null;
  return function(this: any, ...args: any[]) {
    clearTimeout(t);
    t = setTimeout(()=> fn.apply(this, args), wait);
  } as unknown as F;
}

const _PAD6 = `
${Array(600).fill('// filler line to increase filesize for substrate lib6').join('\n')}
`;

export const PAD6 = _PAD6;
