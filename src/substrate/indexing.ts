// indexing.ts - simple in-memory inverted index for text fields
export class InvertedIndex {
  private index = new Map<string, Set<number>>();
  private docs = new Map<number, string>();
  addDoc(id:number, text:string){ this.docs.set(id,text); const toks = tokenize(text); for(const t of toks){ if(!this.index.has(t)) this.index.set(t, new Set()); this.index.get(t)!.add(id); } }
  removeDoc(id:number){ const txt = this.docs.get(id); if(!txt) return; const toks = tokenize(txt); for(const t of toks) { const s = this.index.get(t); if(!s) continue; s.delete(id); if(!s.size) this.index.delete(t); } this.docs.delete(id); }
  search(term:string): number[] { const t = term.toLowerCase(); if(this.index.has(t)) return Array.from(this.index.get(t)!); return []; }
  suggest(prefix:string, limit=10): string[]{ const out:string[]=[]; for(const k of this.index.keys()){ if(k.startsWith(prefix)) out.push(k); if(out.length>=limit) break; } return out; }
  dump(): Record<string, number[]>{ const out:any={}; for(const [k,set] of this.index.entries()) out[k]=Array.from(set); return out; }
}

function tokenize(text:string): string[]{ if(!text) return []; return text.toLowerCase().split(/[^a-z0-9]+/).filter(Boolean); }
