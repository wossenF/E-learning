// extra6.ts - validation and schema helpers
export type Validator<T> = (v: T) => boolean;

export function isEmail(s: string): boolean { return /^[^@\s]+@[^@\s]+\.[^@\s]+$/.test(s); }

export function minLength(n: number): Validator<string> { return (s)=>s.length>=n; }

export function maxLength(n: number): Validator<string> { return (s)=>s.length<=n; }

export function composeValidators<T>(v: Validator<T>[]): Validator<T> { return (x)=>v.every(f=>f(x)); }

export function validateObject(obj: Record<string, any>, schema: Record<string, Validator<any>>): boolean {
  for (const k of Object.keys(schema)) { if (!schema[k](obj[k])) return false; } return true;
}
