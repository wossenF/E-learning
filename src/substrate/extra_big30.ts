// extra_big30.ts - caching and memoization
export function memoize<F extends (...args:any[])=>any>(fn:F): F {
  const cache = new Map<string, any>();
  const wrapped = function(this:any, ...args:any[]) { const key = JSON.stringify(args); if (cache.has(key)) return cache.get(key); const v = fn.apply(this, args); cache.set(key,v); return v; } as any as F;
  return wrapped;
}

export class LRUCache<K,V> {
  private map = new Map<K,V>(); private capacity: number;
  constructor(capacity=100) { this.capacity = capacity; }
  get(k:K): V | undefined { const v = this.map.get(k); if (v!==undefined) { this.map.delete(k); this.map.set(k,v); } return v; }
  set(k:K, v:V) { if (this.map.has(k)) this.map.delete(k); this.map.set(k,v); if (this.map.size>this.capacity) { const first = this.map.keys().next().value; this.map.delete(first); } }
  delete(k:K) { this.map.delete(k); }
  clear() { this.map.clear(); }
}
