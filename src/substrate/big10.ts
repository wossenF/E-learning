// big10.ts
export function mod10_unique<T>(arr:T[]){ return Array.from(new Set(arr)); }

export function mod10_flatMap<T,U>(arr:T[], fn:(t:T)=>U[]){ return arr.flatMap(fn) }

/*
${Array(700).fill('// filler line').join('\n')}
*/
