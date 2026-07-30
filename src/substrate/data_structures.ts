// data_structures.ts - common in-memory data structures (Trie, Heap, UnionFind)

export class TrieNode { children: Map<string, TrieNode> = new Map(); isWord = false; }
export class Trie {
  root = new TrieNode();
  insert(word:string){ let cur=this.root; for(const ch of word){ if(!cur.children.has(ch)) cur.children.set(ch, new TrieNode()); cur = cur.children.get(ch)!; } cur.isWord = true; }
  contains(word:string){ let cur=this.root; for(const ch of word){ if(!cur.children.has(ch)) return false; cur = cur.children.get(ch)!; } return cur.isWord; }
  startsWith(prefix:string){ let cur=this.root; for(const ch of prefix){ if(!cur.children.has(ch)) return []; cur = cur.children.get(ch)!; } const out:string[]=[]; function dfs(node:TrieNode, path:string){ if(node.isWord) out.push(path); for(const [k,n] of node.children) dfs(n, path+k); } dfs(cur, prefix); return out; }
}

export class MinHeap<T> {
  private data:T[] = []; private cmp:(a:T,b:T)=>number;
  constructor(cmp:(a:T,b:T)=>number){ this.cmp = cmp; }
  push(x:T){ this.data.push(x); this.siftUp(this.data.length-1); }
  pop():T|undefined{ if(!this.data.length) return undefined; const r=this.data[0]; const last=this.data.pop()!; if(this.data.length) { this.data[0]=last; this.siftDown(0); } return r; }
  private siftUp(i:number){ while(i>0){ const p=Math.floor((i-1)/2); if(this.cmp(this.data[i], this.data[p])<0){ [this.data[i],this.data[p]]=[this.data[p],this.data[i]]; i=p; } else break; } }
  private siftDown(i:number){ const n=this.data.length; while(true){ let l=2*i+1; let r=2*i+2; let smallest=i; if(l<n && this.cmp(this.data[l], this.data[smallest])<0) smallest=l; if(r<n && this.cmp(this.data[r], this.data[smallest])<0) smallest=r; if(smallest===i) break; [this.data[i],this.data[smallest]]=[this.data[smallest],this.data[i]]; i=smallest; } }
  size(){ return this.data.length; }
}

export class UnionFind {
  parent: number[]; rank: number[];
  constructor(n:number){ this.parent = new Array(n); this.rank = new Array(n); for(let i=0;i<n;i++){ this.parent[i]=i; this.rank[i]=0; } }
  find(x:number){ if(this.parent[x]!==x) this.parent[x] = this.find(this.parent[x]); return this.parent[x]; }
  union(a:number,b:number){ const ra=this.find(a), rb=this.find(b); if(ra===rb) return false; if(this.rank[ra]<this.rank[rb]) this.parent[ra]=rb; else if(this.rank[rb]<this.rank[ra]) this.parent[rb]=ra; else { this.parent[rb]=ra; this.rank[ra]++; } return true; }
}

export function flatten<T>(arr:T[][]): T[]{ const out:T[]=[]; for(const a of arr) out.push(...a); return out; }
