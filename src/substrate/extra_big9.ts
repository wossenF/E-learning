// extra_big9.ts - more fake IO and helpers
export async function writeFile(path:string, data:string): Promise<void> { /* fake */ return Promise.resolve(); }
export async function readFile(path:string): Promise<string> { return Promise.resolve(''); }

export function toBase64(s:string): string { return Buffer.from(s).toString('base64'); }
export function fromBase64(s:string): string { return Buffer.from(s,'base64').toString('utf8'); }

export function splitLines(s:string): string[] { return s.split(/\r?\n/); }
