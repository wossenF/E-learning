// lib10.ts
export function toTitleCase(s:string){ return s.replace(/\w\S*/g, (txt)=>txt.charAt(0).toUpperCase()+txt.substr(1).toLowerCase()); }

export function slugify(s:string){ return s.toLowerCase().replace(/[^a-z0-9]+/g,'-').replace(/^-+|-+$/g,''); }

const _PAD10 = `
${Array(600).fill('// filler line to increase filesize for substrate lib10').join('\n')}
`;

export const PAD10 = _PAD10;
