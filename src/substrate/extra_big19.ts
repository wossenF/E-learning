// extra_big19.ts - query mini-language for DataFrame
import { DataFrame, Row } from './extra_big16';

export type Expr = { op: 'const'|'col'|'eq'|'and'|'or'|'gt'|'lt'|'gte'|'lte'|'like', value?: any, col?: string, left?:Expr, right?:Expr };

export function evalExpr(e: Expr, row: Row): boolean {
  switch (e.op) {
    case 'const': return Boolean(e.value);
    case 'col': return Boolean(row[e.col!]);
    case 'eq': return row[e.left!.col!] == (e.right!.value ?? row[e.right!.col!]);
    case 'gt': return Number(row[e.left!.col!]) > Number(e.right!.value ?? row[e.right!.col!]);
    case 'lt': return Number(row[e.left!.col!]) < Number(e.right!.value ?? row[e.right!.col!]);
    case 'gte': return Number(row[e.left!.col!]) >= Number(e.right!.value ?? row[e.right!.col!]);
    case 'lte': return Number(row[e.left!.col!]) <= Number(e.right!.value ?? row[e.right!.col!]);
    case 'and': return evalExpr(e.left!,row) && evalExpr(e.right!,row);
    case 'or': return evalExpr(e.left!,row) || evalExpr(e.right!,row);
    case 'like': {
      const v = String(row[e.left!.col!]); const pat = String(e.right!.value); return v.indexOf(pat) >= 0;
    }
    default: return false;
  }
}

export function filterByExpr(df: DataFrame, expr: Expr): DataFrame { return df.filter(r=>evalExpr(expr,r)); }

export function parseSimple(query: string): Expr | null {
  // very small parser supporting: col == value, col > value, col like value, and/ or
  const tokens = query.replace(/\s+/g,' ').trim().split(' ');
  if (tokens.length<3) return null;
  // handle simple forms: a == b
  if (tokens.includes('==')) {
    const i = tokens.indexOf('=='); return { op:'eq', left:{op:'col', col:tokens[i-1]}, right:{op:'const', value: tokens[i+1].replace(/^"|"$/g,'') } };
  }
  if (tokens.includes('>')) { const i = tokens.indexOf('>'); return {op:'gt', left:{op:'col', col:tokens[i-1]}, right:{op:'const', value: Number(tokens[i+1])}}; }
  if (tokens.includes('like')) { const i = tokens.indexOf('like'); return {op:'like', left:{op:'col', col:tokens[i-1]}, right:{op:'const', value: tokens[i+1].replace(/^"|"$/g,'')} }; }
  return null;
}
