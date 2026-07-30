// big1.ts - generated filler module to expand first-party source
export function mod1_funcA(n: number): number {
  let r = 1;
  for (let i = 1; i <= n; i++) r = (r + i) % 100000;
  return r;
}

export function mod1_funcB(s: string): string {
  return s.split('').reverse().join('') + '_B';
}

// filler lines to increase file size
//
//
//
//
// filler

/*
${Array(700).fill('// filler line').join('\n')}
*/
