// lib1.ts - utility functions (generated filler to increase substrate size)
export function greet(name: string): string {
  return `Hello, ${name}!`;
}

export function factorial(n: number): number {
  if (n <= 1) return 1;
  let res = 1;
  for (let i = 2; i <= n; i++) res *= i;
  return res;
}

export function hashString(s: string): number {
  let h = 2166136261;
  for (let i = 0; i < s.length; i++) {
    h ^= s.charCodeAt(i);
    h = Math.imul(h, 16777619);
  }
  return h >>> 0;
}

// Add repeated code to increase file size
const _PAD1 = `
${Array(500).fill('/* filler line to increase filesize for substrate */').join('\n')}
`;

export const PAD1 = _PAD1;
