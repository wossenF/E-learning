// big5.ts
export function mod5_obj_keys(obj: Record<string, any>): string[] { return Object.keys(obj); }

export function mod5_clone<T>(o:T):T { return JSON.parse(JSON.stringify(o)); }

/*
${Array(700).fill('// filler line').join('\n')}
*/
