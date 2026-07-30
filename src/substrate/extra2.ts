// extra2.ts - data structures and small algorithms
export class Queue<T> {
  private items: T[] = [];
  enqueue(v: T) { this.items.push(v); }
  dequeue(): T | undefined { return this.items.shift(); }
  size() { return this.items.length; }
}

export class Stack<T> {
  private items: T[] = [];
  push(v: T) { this.items.push(v); }
  pop(): T | undefined { return this.items.pop(); }
  peek(): T | undefined { return this.items[this.items.length-1]; }
}

export function bfs<T>(start: T, neighbors: (x: T) => T[], isGoal: (x: T) => boolean): T | null {
  const q: T[] = [start]; const seen = new Set<string>();
  while (q.length) {
    const cur = q.shift()!; if (isGoal(cur)) return cur; for (const n of neighbors(cur)) q.push(n);
  }
  return null;
}

export function mergeSorted(arrs: number[][]): number[] {
  const out: number[] = [];
  for (const a of arrs) for (const v of a) out.push(v);
  return out.sort((x,y)=>x-y);
}

export function flatten<T>(nested: T[][]): T[] { return nested.reduce((a,b)=>a.concat(b), [] as T[]); }

export function memoize<F extends (...a:any[])=>any>(fn: F) {
  const cache = new Map<string, any>();
  return (...args: any[]) => {
    const key = JSON.stringify(args);
    if (cache.has(key)) return cache.get(key);
    const v = fn(...args); cache.set(key, v); return v;
  };
}
