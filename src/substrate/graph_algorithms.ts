// graph_algorithms.ts - common graph algorithms (BFS, DFS, Dijkstra, Topological sort)
export type AdjList = Map<string, Array<{to:string, weight?:number}>>;

export function bfs(adj:AdjList, start:string): string[] {
  const seen = new Set<string>(); const q:string[] = [start]; const out:string[] = [];
  seen.add(start);
  while(q.length){ const u = q.shift()!; out.push(u); const nbrs = adj.get(u) || []; for(const e of nbrs){ if(!seen.has(e.to)){ seen.add(e.to); q.push(e.to); } } }
  return out;
}

export function dfs(adj:AdjList, start:string): string[] {
  const seen = new Set<string>(); const out:string[]=[];
  function visit(u:string){ if(seen.has(u)) return; seen.add(u); out.push(u); for(const e of adj.get(u)||[]) visit(e.to); }
  visit(start); return out;
}

export function dijkstra(adj:AdjList, source:string): Map<string, number> {
  const dist = new Map<string, number>(); const pq: Array<{node:string, dist:number}> = [];
  function push(n:string,d:number){ pq.push({node:n,dist:d}); pq.sort((a,b)=>a.dist-b.dist); }
  push(source,0); while(pq.length){ const cur = pq.shift()!; if(dist.has(cur.node)) continue; dist.set(cur.node, cur.dist); for(const e of adj.get(cur.node) || []){ if(!dist.has(e.to)) push(e.to, cur.dist + (e.weight||1)); } }
  return dist;
}

export function topoSort(adj:AdjList): string[] | null {
  const indeg = new Map<string, number>(); for(const [u,edges] of adj.entries()){ if(!indeg.has(u)) indeg.set(u,0); for(const e of edges){ indeg.set(e.to, (indeg.get(e.to)||0)+1); } }
  const q:string[] = []; for(const [k,v] of indeg.entries()) if(v===0) q.push(k);
  const out:string[] = [];
  while(q.length){ const u=q.shift()!; out.push(u); for(const e of adj.get(u)||[]) { indeg.set(e.to, (indeg.get(e.to)||0)-1); if(indeg.get(e.to)===0) q.push(e.to); } }
  if(out.length !== indeg.size) return null; return out;
}

export function stronglyConnected(adj:AdjList): string[][] {
  // Kosaraju
  const order:string[] = []; const seen=new Set<string>(); function dfs1(u:string){ seen.add(u); for(const e of adj.get(u)||[]) if(!seen.has(e.to)) dfs1(e.to); order.push(u); }
  for(const k of adj.keys()) if(!seen.has(k)) dfs1(k);
  const rev = new Map<string, Array<{to:string,weight?:number}>>(); for(const [u,edges] of adj.entries()){ for(const e of edges){ if(!rev.has(e.to)) rev.set(e.to,[]); rev.get(e.to)!.push({to:u,weight:e.weight}); } }
  const comps:string[][] = []; seen.clear(); function dfs2(u:string, comp:string[]){ seen.add(u); comp.push(u); for(const e of rev.get(u)||[]) if(!seen.has(e.to)) dfs2(e.to, comp); }
  for(let i=order.length-1;i>=0;--i){ const u=order[i]; if(!seen.has(u)){ const comp:string[]=[]; dfs2(u, comp); comps.push(comp); } }
  return comps;
}

export function buildAdj(edges: Array<[string,string,number?]>): AdjList { const m = new Map<string, Array<{to:string, weight?:number}>>(); for(const [a,b,w] of edges){ if(!m.has(a)) m.set(a,[]); m.get(a)!.push({to:b, weight:w}); if(!m.has(b)) m.set(b, []); } return m; }
