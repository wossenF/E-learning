// extra_big16.ts - mini DataFrame implementation
export type Row = Record<string, any>;

export class DataFrame {
  private rows: Row[];
  constructor(rows: Row[] = []) { this.rows = rows.slice(); }
  add(row: Row) { this.rows.push(row); }
  addAll(rows: Row[]) { this.rows.push(...rows); }
  count(): number { return this.rows.length; }
  getRows(): Row[] { return this.rows.slice(); }

  select(cols: string[]): DataFrame {
    const out = this.rows.map(r => { const o: Row = {}; for (const c of cols) o[c] = r[c]; return o; });
    return new DataFrame(out);
  }

  filter(pred: (r:Row)=>boolean): DataFrame {
    return new DataFrame(this.rows.filter(pred));
  }

  map(fn: (r:Row)=>Row): DataFrame { return new DataFrame(this.rows.map(fn)); }

  groupBy(keys: string[]): GroupedDataFrame {
    const map = new Map<string, Row[]>();
    for (const r of this.rows) {
      const k = keys.map(kk => String(r[kk])).join('\u0001');
      if (!map.has(k)) map.set(k, []);
      map.get(k)!.push(r);
    }
    return new GroupedDataFrame(keys, map);
  }

  orderBy(col: string, asc=true): DataFrame {
    const out = this.rows.slice().sort((a,b)=>{
      const va = a[col]; const vb = b[col]; if (va==vb) return 0; if (va===undefined) return 1; if (vb===undefined) return -1; return (va<vb? -1:1) * (asc?1:-1);
    });
    return new DataFrame(out);
  }

  limit(n:number): DataFrame { return new DataFrame(this.rows.slice(0,n)); }

  toJSON(): string { return JSON.stringify(this.rows); }
}

export class GroupedDataFrame {
  constructor(public keys: string[], private groups: Map<string, Row[]>) {}
  agg(aggs: Record<string, (rows:Row[])=>any>): DataFrame {
    const out: Row[] = [];
    for (const [k,rows] of this.groups.entries()) {
      const keyParts = k.split('\u0001'); const base: any = {};
      for (let i=0;i<this.keys.length;++i) base[this.keys[i]] = keyParts[i];
      for (const name of Object.keys(aggs)) base[name] = aggs[name](rows);
      out.push(base);
    }
    return new DataFrame(out);
  }
}

// helpers
export function sum(rows: Row[], col:string): number { return rows.reduce((s,r)=>s + (Number(r[col])||0), 0); }
export function avg(rows: Row[], col:string): number { if (!rows.length) return 0; return sum(rows,col)/rows.length; }
export function min(rows: Row[], col:string): number { let m = Infinity; for (const r of rows) { const v=Number(r[col]); if (!isNaN(v) && v<m) m=v; } return m===Infinity? 0: m; }
export function max(rows: Row[], col:string): number { let m = -Infinity; for (const r of rows) { const v=Number(r[col]); if (!isNaN(v) && v>m) m=v; } return m===-Infinity? 0: m; }

export function fromArrayOfObjects(arr: Row[]): DataFrame { return new DataFrame(arr); }

// small example generator used by tests and demos
export function exampleData(n=50): DataFrame {
  const rows: Row[] = [];
  for (let i=0;i<n;++i) rows.push({id:i, group: i%5, value: Math.floor(Math.random()*100), ts: Date.now() + i});
  return new DataFrame(rows);
}
